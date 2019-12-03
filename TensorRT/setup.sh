#!/bin/bash -e

cd $WORK

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
patch -p1 < $CMSSW_BASE/src/SonicCMS/TensorRT/patch.diff
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
cat << 'EOF_TOOLFILE' > tensorrt.xml
<tool name="tensorrtis" version="v19.10">
  <info url="https://github.com/NVIDIA/tensorrt-inference-server"/>
  <lib name="request"/> 
  <client>             
    <environment name="TENSORRTIS_BASE" default="$CMSSW_BASE/work/local/tensorrtis"/>
    <environment name="INCLUDE" default="$TENSORRTIS_BASE/include"/>
    <environment name="LIBDIR"  default="$TENSORRTIS_BASE/lib"/>
</client>                                                                                                                                                                                                                                
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

mv tensorrt.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
mv protobuf.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup tensorrt
scram setup protobuf

# remove the huge source code directory and intermediate products that are not needed to run
if [ -z "$DEBUG" ]; then
	cd $WORK
	rm -rf tensorrt-inference-server
fi
