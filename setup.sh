#!/bin/bash

export SCRAM_ARCH=slc6_amd64_gcc700
# cmsrel
CMSSWVER=CMSSW_10_2_0
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

# download and build grpc
INSTALLDIR=work/local/grpc
cd $WORK
git clone https://github.com/grpc/grpc -b v1.14.0
cd grpc
git submodule update --init
make -j 8
# get ldconfig
if ! [ type ldconfig >& /dev/null ]; then
	export PATH=${PATH}:/sbin
fi
# get around "make: execvp: /bin/sh: Argument list too long" if directory path is long
ln -s $CMSSW_BASE base
make install prefix=base/$INSTALLDIR
# link libraries because tensorflow_serving tries to get the wrong versions for some reason
cd $CMSSW_BASE/$INSTALLDIR/lib
ln -s libgrpc++.so.1.14.0 libgrpc++.so.1
ln -s libgrpc++_reflection.so.1.14.0 libgrpc++_reflection.so.1
cd -

# setup in scram
cat << 'EOF_TOOLFILE' > grpc.xml
<tool name="grpc" version="v1.14.0">
  <info url="https://github.com/grpc/grpc"/>
  <lib name="grpc"/>
  <lib name="grpc++"/>
  <lib name="grpc++_reflection"/>
  <client>
    <environment name="GRPC_BASE" default="$CMSSW_BASE/$INSTALLDIR"/>
    <environment name="INCLUDE" default="$GRPC_BASE/include"/>
    <environment name="LIBDIR" default="$GRPC_BASE/lib"/>
  </client>
</tool>
EOF_TOOLFILE
sed -i 's~$INSTALLDIR~'$INSTALLDIR'~' grpc.xml

mv grpc.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup grpc

# download and build tensorflow_serving w/ cmake
INSTALLDIR=work/local/tensorflow_serving
cd $WORK
git clone https://github.com/hls-fpga-machine-learning/inception_cmake
cd inception_cmake
git submodule update --init
cd serving
git checkout 1.6.1
git clone --recursive https://github.com/tensorflow/tensorflow.git -b v1.6.0
cd ..
# to get cmake
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.7.0/Linux-x86_64/bin/:${PATH}
mkdir build
cd build
# some really bad ways to get info out of scram
PROTOBUF_LIBDIR=$(scram tool info protobuf | grep "LIBDIR=" | sed 's/LIBDIR=//')
PROTOBUF_INCLUDE=$(scram tool info protobuf | grep "INCLUDE=" | sed 's/INCLUDE=//')
GRPC_LIBDIR=$(scram tool info grpc | grep "LIBDIR=" | sed 's/LIBDIR=//')
GRPC_INCLUDE=$(scram tool info grpc | grep "INCLUDE=" | sed 's/INCLUDE=//')
GRPC_BIN=$(scram tool info grpc | grep "GRPC_BASE=" | sed 's/GRPC_BASE=//')
TENSORFLOW_LIBDIR=$(scram tool info tensorflow | grep "LIBDIR=" | sed 's/LIBDIR=//')
TENSORFLOW_INCLUDE=$(scram tool info tensorflow | grep "INCLUDE=" | sed 's/INCLUDE=//')
cmake .. -DPROTOBUF_LIBRARY=$PROTOBUF_LIBDIR/libprotobuf.so -DPROTOBUF_INCLUDE_DIR=$PROTOBUF_INCLUDE -DGRPC_LIBRARY=$GRPC_LIBDIR/libgrpc.so -DGRPC_GRPC++_LIBRARY=$GRPC_LIBDIR/libgrpc++.so -DGRPC_INCLUDE_DIR=$GRPC_INCLUDE -DGRPC_GRPC++_REFLECTION_LIBRARY=$GRPC_LIBDIR/libgrpc++_reflection.so -DGRPC_CPP_PLUGIN=$GRPC_BIN/bin/grpc_cpp_plugin -DTENSORFLOW_CC_LIBRARY=$TENSORFLOW_LIBDIR/libtensorflow_cc.so -DTENSORFLOW_INCLUDE_DIR=$TENSORFLOW_INCLUDE -DEIGEN_INCLUDE_DIR=$TENSORFLOW_INCLUDE/eigen -DTENSORFLOW_FWK_LIBRARY=$TENSORFLOW_LIBDIR/libtensorflow_framework.so
make
# install
mkdir $CMSSW_BASE/$INSTALLDIR
mkdir $CMSSW_BASE/$INSTALLDIR/lib
mkdir $CMSSW_BASE/$INSTALLDIR/include
cp libtfserving.so $CMSSW_BASE/$INSTALLDIR/lib/
cp -r proto-src/tensorflow_serving $CMSSW_BASE/$INSTALLDIR/include/

# setup in scram
cat << 'EOF_TOOLFILE' > tensorflow-serving.xml
<tool name="tensorflow-serving" version="1.6.1">
  <info url="https://github.com/kpedro88/inception_cmake"/>
  <lib name="tfserving"/>
  <client>
    <environment name="TFSERVING_BASE" default="$CMSSW_BASE/$INSTALLDIR"/>
    <environment name="INCLUDE" default="$TFSERVING_BASE/include"/>
    <environment name="LIBDIR" default="$TFSERVING_BASE/lib"/>
  </client>
  <use name="protobuf"/>
  <use name="grpc"/>
  <use name="eigen"/>
  <use name="tensorflow-cc"/>
  <use name="tensorflow-framework"/>
</tool>
EOF_TOOLFILE
sed -i 's~$INSTALLDIR~'$INSTALLDIR'~' tensorflow-serving.xml

mv tensorflow-serving.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup tensorflow-serving

# setup for conda environment (kept separate from CMSSW)
cd $CMSSW_BASE/..

# setup miniconda
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh -b -p $CMSSW_BASE/../miniconda3
source /uscms_data/d3/pedrok/phase2/brainwave/test/miniconda3/etc/profile.d/conda.sh

# setup aml
git clone https://github.com/Azure/aml-real-time-ai
miniconda3/bin/conda env create -f aml-real-time-ai/environment.yml

# to get working version
AMLDIR=miniconda3/envs/amlrealtimeai/lib/python3.6/site-packages
mv $AMLDIR/tensorflow $AMLDIR/tensorflow-bak
ln -s $CMSSWTF $AMLDIR/tensorflow

# get the analysis code
cd $CMSSW_BASE/src
git cms-init
git clone git@github.com:hls-fpga-machine-learning/SonicCMS -b "kjp/1020_azureml"
scram b -j 8
