#!/bin/bash -e

FORK=hls-fpga-machine-learning
BRANCH=abstract
CMSSWVER=CMSSW_10_6_6
ACCESS=https
PKGS=()
PKGS_ALL=(
TensorRT \
)

# these are used by pkg-specific setup scripts
export CORES=1
export DEBUG=""

join_by() { local IFS="$1"; shift; echo "$*"; }

usage(){
	EXIT=$1

	echo "setup.sh [options]"
	echo ""
	echo "-f [fork]           clone from specified fork (default = ${FORK})"
	echo "-b [branch]         clone specified branch (default = ${BRANCH})"
	echo "-c [version]        use specified CMSSW version (default = ${CMSSWVER})"
	echo "-p [packages]       set up specified packages (allowed = "$(join_by , "${PKGS_ALL[@]}")"; or all)"
	echo "-a [protocol]       use protocol to clone (default = ${ACCESS}, alternative = ssh)"
	echo "-j [cores]          run compilations on # cores (default = ${CORES})"
	echo "-d                  keep source code for debugging"
	echo "-h                  display this message and exit"

	exit $EXIT
}

# process options
while getopts "f:b:c:p:a:j:dh" opt; do
	case "$opt" in
	f) FORK=$OPTARG
	;;
	b) BRANCH=$OPTARG
	;;
	c) CMSSWVER=$OPTARG
	;;
	p) if [ "$OPTARG" = all ]; then PKGS=(${PKGS_ALL[@]}); else IFS="," read -a PKGS <<< "$OPTARG"; fi
	;;
	a) ACCESS=$OPTARG
	;;
	j) export CORES=$OPTARG
	;;
	d) export DEBUG=true
	;;
	h) usage 0
	;;
	esac
done

# check options
if [ "$ACCESS" = "ssh" ]; then
	export ACCESS_GITHUB=git@github.com:
	export ACCESS_CMSSW=--ssh
elif [ "$ACCESS" = "https" ]; then
	export ACCESS_GITHUB=https://github.com/
	export ACCESS_CMSSW=--https
else
	usage 1
fi

export SCRAM_ARCH=slc7_amd64_gcc700
# cmsrel
scram project ${CMSSWVER}
cd ${CMSSWVER}
eval `scramv1 runtime -sh`

# prepare for installation
unset PYTHONPATH
export WORK=$CMSSW_BASE/work
mkdir $WORK
cd $WORK
export LOCAL=$WORK/local
mkdir $LOCAL

# clone sonic
cd $CMSSW_BASE/src
git cms-init $ACCESS_CMSSW
git clone ${ACCESS_GITHUB}${FORK}/SonicCMS -b ${BRANCH}
git clone ${ACCESS_GITHUB}kpedro88/CondorProduction Condor/Production

# setup sparse checkout for specified packages
cd $CMSSW_BASE/src/SonicCMS
SPARSE=.git/info/sparse-checkout
cat << EOF_SPARSE > ${SPARSE}
/Core
/*.*
EOF_SPARSE
for PKG in ${PKGS[@]}; do
	echo "/$PKG" >> ${SPARSE}
done
git read-tree -mu HEAD

# loop through package-specific installations
for PKG in ${PKGS[@]}; do
	${PKG}/setup.sh
done

# build CMSSW
cd $CMSSW_BASE/src
# update env just in case
eval `scramv1 runtime -sh`
scram b -j ${CORES}

# setup extra python libs
NEWPY2PATH=$LOCAL/lib/python2.7/site-packages
mkdir -p $NEWPY2PATH
# need --ignore-installed or it tries to uninstall cvmfs packages
pip install --prefix $LOCAL --ignore-installed wheel setuptools grpcio-tools
export PYTHON27PATH=$NEWPY2PATH:$PYTHON27PATH

# to get cmake
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.7.0/Linux-x86_64/bin/:${PATH}

# some really bad ways to get info out of scram
PYTHON_LIBDIR=$(scram tool info python | grep "LIBDIR=" | sed 's/LIBDIR=//')
PYTHON_INCLUDE=$(scram tool info python | grep "INCLUDE=" | sed 's/INCLUDE=//')

# download and build tensor-rt-inference server
git clone ${ACCESS_GITHUB}NVIDIA/tensorrt-inference-server.git -b r19.10
cd tensorrt-inference-server
mkdir workspace
cd workspace
cp ../VERSION .
cp -r ../build .
mkdir install
mkdir src
cp -r ../src/clients src
cp -r ../src/core src
cd build

git clone ${ACCESS_GITHUB}opencv/opencv.git
cd opencv/
wget https://raw.githubusercontent.com/hls-fpga-machine-learning/SonicCMS/pch/gpu/patch.diff
patch -p1 < patch.diff
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ../ -DPYTHON_LIBRARY=$PYTHON_LIBDIR -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE
make -j $CORES

cd ../../
export Protobuf_DIR="$PWD/protobuf/lib64/cmake/protobuf/"
export OpenCV_DIR="$PWD/opencv/build/"
export CURL_DIR="$PWD/curl/install/lib64/cmake/CURL/"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../install
make -j $CORES trtis-clients
cd ../
cp -r build/install ${CMSSW_BASE}/work/local/tensorrtis
cp -r build/protobuf $CMSSW_BASE/work/local/protobuf

# setup in scram
cat << 'EOF_TOOLFILE' > tensorrtis.xml
<tool name="tensorrtis" version="v19.10">
  <info url="https://github.com/NVIDIA/tensorrt-inference-server"/>
  <lib name="request"/> 
  <client>             
    <environment name="TENSORRTIS_BASE" default="$CMSSW_BASE/work/local/tensorrtis"/>
    <environment name="INCLUDE" default="$TENSORRTIS_BASE/include"/>
    <environment name="LIBDIR"  default="$TENSORRTIS_BASE/lib"/>
  </client>
  <use name="protobuf"/>
</tool>               
EOF_TOOLFILE

cat << 'EOF_TOOLFILE' > protobuf.xml
<tool name="protobuf" version="3.5.1">
  <lib name="protobuf"/>
  <client>
    <environment name="PROTOBUF_BASE" default="$CMSSW_BASE/work/local/protobuf"/>
    <environment name="INCLUDE" default="$PROTOBUF_BASE/include"/>
    <environment name="LIBDIR"  default="$PROTOBUF_BASE/lib64"/>
    <environment name="BINDIR"  default="$PROTOBUF_BASE/bin"/>
  </client>
  <runtime name="PATH" value="$PROTOBUF_BASE/bin" type="path"/>
  <runtime name="ROOT_INCLUDE_PATH" value="$INCLUDE" type="path"/>
</tool>
EOF_TOOLFILE

mv tensorrtis.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
mv protobuf.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup protobuf
scram setup tensorrtis

# remove the huge source code directory and intermediate products that are not needed to run
if [ -z "$DEBUG" ]; then
	cd $WORK
	rm -rf tensorrt-inference-server
fi

# get the analysis code
cd $CMSSW_BASE/src
git cms-init $ACCESS_CMSSW
git clone ${ACCESS_GITHUB}hls-fpga-machine-learning/SonicCMS -b "pch/gpu"
git clone ${ACCESS_GITHUB}kpedro88/CondorProduction Condor/Production
scram b -j $CORES

