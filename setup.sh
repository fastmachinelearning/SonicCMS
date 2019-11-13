#!/bin/bash

export SCRAM_ARCH=slc7_amd64_gcc700
# cmsrel
CMSSWVER=CMSSW_10_6_1_patch3
scram project ${CMSSWVER}
cd ${CMSSWVER}
# cmsenv
eval `scramv1 runtime -sh`

unset PYTHONPATH
CMSSWTF=$(dirname $(python3 -c "import tensorflow; print(tensorflow.__file__)"))
WORK=$CMSSW_BASE/work
mkdir $WORK
cd $WORK
LOCAL=$WORK/local
mkdir $LOCAL

# setup miniconda
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh -b -p $CMSSW_BASE/../miniconda3
source $CMSSW_BASE/../miniconda3/etc/profile.d/conda.sh
conda activate
pip install --upgrade wheel setuptools grpcio-tools

# download and build tensor-rt-inference server
git clone https://github.com/NVIDIA/tensorrt-inference-server.git
cd tensorrt-inference-server
git checkout r19.10
mkdir workspace
cd workspace
cp ../VERSION .
cp -r ../build .
mkdir install
mkdir src
cp -r ../src/clients src
cp -r ../src/core src
cd build
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.7.0/Linux-x86_64/bin/:${PATH}
git clone https://github.com/opencv/opencv.git
cd opencv/
mkdir build
cmake -D CMAKE_BUILD_TYPE=Release ../
find ./ -type f -exec sed -i 's/#ifdef HAVE_TIFF/#ifndef HAVE_TIFF/g' {} \;
make -j7
cd ..
export Protobuf_DIR="$PWD/protobuf/lib64/cmake/protobuf/"
export OpenCV_DIR="$PWD/opencv/build/"
export CURL_DIR="$PWD/curl/install/lib64/cmake/CURL/"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../install
make -j16 trtis-clients
cd ../
cp -r install ${CMSSW_BASE}/work/local/tensorrtis
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
<tool name="protobuf" version="3.5.2-pafccj">
  <lib name="protobuf"/>
  <client>
    <environment name="TENSORRTIS_BASE" default="$CMSSW_BASE/work/local/protobuf"/>
    <environment name="INCLUDE" default="$TENSORRTIS_BASE/include"/>
    <environment name="LIBDIR"  default="$TENSORRTIS_BASE/lib64"/>
    <environment name="BINDIR"  default="$TENSORRTIS_BASE/bin"/>
  </client>
</tool>
EOF_TOOLFILE

cp tensorrt.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
cp protobuf.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup tensorrt
scram setup protobuf

# get the analysis code
cd $CMSSW_BASE/src
git cms-init
git clone https://github.com/hls-fpga-machine-learning/SonicCMS -b "pch/tensorrt"
git clone https://github.com/kpedro88/CondorProduction Condor/Production
scram b -j 8

