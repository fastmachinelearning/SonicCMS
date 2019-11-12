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
cd build
cmake -D CMAKE_BUILD_TYPE=Release ../
make -j7
cd ..
export Protobuf_DIR="$PWD/protobuf/lib64/cmake/protobuf/"
export OpenCV_DIR="$PWD/opencv/build/"
export CURL_DIR="$PWD/curl/install/lib64/cmake/CURL/"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../install

#You may need something from below
#in  trtis-clients/tmp/trtis-clients-cache-Release.cmake modified for workspace /data/t3home000/pharris/sonic2/tensorrt-inference-server/build/../../workspace/
cd ../
cp -r install ${CMSSW_BASE}/work/local/tensorrtis
cp -r build/protobuf $CMSSW_BASE/work/local/protobuf
cp tensorrt.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
cp protobuf.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup tensorrt
scram setup protobuf

# to get cmake
#
# some really bad ways to get info out of scram
#PROTOBUF_LIBDIR=$(scram tool info protobuf | grep "LIBDIR=" | sed 's/LIBDIR=//')
#PROTOBUF_INCLUDE=$(scram tool info protobuf | grep "INCLUDE=" | sed 's/INCLUDE=//')
#TENSORFLOW_LIBDIR=$(scram tool info tensorflow | grep "LIBDIR=" | sed 's/LIBDIR=//')
#TENSORFLOW_INCLUDE=$(scram tool info tensorflow | grep "INCLUDE=" | sed 's/INCLUDE=//')


# get the analysis code
cd $CMSSW_BASE/src
git cms-init
git clone https://github.com/hls-fpga-machine-learning/SonicCMS -b "pch/tensorrt"
git clone https://github.com/kpedro88/CondorProduction Condor/Production
scram b -j 8

