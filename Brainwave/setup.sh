#!/bin/bash -e

CMSSWTF=$(dirname $(python3 -c "import tensorflow; print(tensorflow.__file__)"))

# download and build grpc
INSTALLDIR=work/local/grpc
cd $WORK
git clone ${ACCESS_GITHUB}grpc/grpc -b v1.27.3
cd grpc
git submodule update --init
make -j ${CORES}
# get ldconfig
if ! [ type ldconfig >& /dev/null ]; then
	export PATH=${PATH}:/sbin
fi
# get around "make: execvp: /bin/sh: Argument list too long" if directory path is long
ln -s $CMSSW_BASE base
make install prefix=base/$INSTALLDIR

# rename protobuf-bw libraries to avoid collisions
PBDIR=work/local/protobuf-bw/
cd $CMSSW_BASE
mkdir -p $PBDIR/lib64
cp $WORK/grpc/third_party/protobuf/src/.libs/*.a $PBDIR/lib64/
mkdir -p $PBDIR/include
cp -r $WORK/grpc/third_party/protobuf/src/google $PBDIR/include/
mkdir -p $PBDIR/bin
cp $WORK/grpc/third_party/protobuf/src/protoc $PBDIR/bin/
cd $PBDIR/lib64
mv libprotobuf-lite.a libprotobuf-bw-lite.a
mv libprotobuf.a libprotobuf-bw.a
mv libprotoc.a libprotoc-bw.a

cd $WORK
# setup in scram
cat << 'EOF_TOOLFILE' > grpc.xml
<tool name="grpc" version="v1.27.3">
  <info url="https://github.com/grpc/grpc"/>
  <lib name="grpc"/>
  <lib name="grpc++"/>
  <lib name="grpc++_reflection"/>
  <client>
    <environment name="GRPC_BASE" default="$CMSSW_BASE/$INSTALLDIR"/>
    <environment name="INCLUDE" default="$GRPC_BASE/include"/>
    <environment name="LIBDIR" default="$GRPC_BASE/lib"/>
  </client>
  <use name="protobuf-bw"/>
</tool>
EOF_TOOLFILE
sed -i 's~$INSTALLDIR~'$INSTALLDIR'~' grpc.xml

# setup as a separate external in case the builtin version is needed
cat << 'EOF_TOOLFILE' > protobuf-bw.xml
<tool name="protobuf-bw" version="3.11.0">
  <lib name="protobuf-bw"/>
  <client>
    <environment name="PROTOBUF_BASE" default="$CMSSW_BASE/work/local/protobuf-bw"/>
    <environment name="INCLUDE" default="$PROTOBUF_BASE/include"/>
    <environment name="LIBDIR"  default="$PROTOBUF_BASE/lib64"/>
    <environment name="BINDIR"  default="$PROTOBUF_BASE/bin"/>
  </client>
  <runtime name="PATH" value="$PROTOBUF_BASE/bin" type="path"/>
  <runtime name="ROOT_INCLUDE_PATH" value="$INCLUDE" type="path"/>
</tool>
EOF_TOOLFILE

mv grpc.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
mv protobuf-bw.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup grpc
scram setup protobuf-bw

# download and build tensorflow_serving w/ cmake
INSTALLDIR=work/local/tensorflow_serving
cd $WORK
git clone ${ACCESS_GITHUB}hls-fpga-machine-learning/inception_cmake
cd inception_cmake
git submodule update --init
cd serving
git checkout 1.6.1
git clone --recursive ${ACCESS_GITHUB}tensorflow/tensorflow.git -b v1.6.0
cd ..
# to get cmake
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.7.0/Linux-x86_64/bin/:${PATH}
mkdir build
cd build
# some really bad ways to get info out of scram
PROTOBUF_BINDIR=$(scram tool info protobuf-bw | grep "BINDIR=" | sed 's/BINDIR=//')
# make sure desired version of protoc executable comes first in the path
export PATH=${PROTOBUF_BINDIR}:${PATH}
PROTOBUF_LIBDIR=$(scram tool info protobuf-bw | grep "LIBDIR=" | sed 's/LIBDIR=//')
PROTOBUF_INCLUDE=$(scram tool info protobuf-bw | grep "INCLUDE=" | sed 's/INCLUDE=//')
GRPC_LIBDIR=$(scram tool info grpc | grep "LIBDIR=" | sed 's/LIBDIR=//')
GRPC_INCLUDE=$(scram tool info grpc | grep "INCLUDE=" | sed 's/INCLUDE=//')
GRPC_BIN=$(scram tool info grpc | grep "GRPC_BASE=" | sed 's/GRPC_BASE=//')
TENSORFLOW_LIBDIR=$(scram tool info tensorflow | grep "LIBDIR=" | sed 's/LIBDIR=//')
TENSORFLOW_INCLUDE=$(scram tool info tensorflow | grep "INCLUDE=" | sed 's/INCLUDE=//')
cmake .. -DPROTOBUF_LIBRARY=$PROTOBUF_LIBDIR/libprotobuf.so -DPROTOBUF_INCLUDE_DIR=$PROTOBUF_INCLUDE -DGRPC_LIBRARY=$GRPC_LIBDIR/libgrpc.so -DGRPC_GRPC++_LIBRARY=$GRPC_LIBDIR/libgrpc++.so -DGRPC_INCLUDE_DIR=$GRPC_INCLUDE -DGRPC_GRPC++_REFLECTION_LIBRARY=$GRPC_LIBDIR/libgrpc++_reflection.so -DGRPC_CPP_PLUGIN=$GRPC_BIN/bin/grpc_cpp_plugin -DTENSORFLOW_CC_LIBRARY=$TENSORFLOW_LIBDIR/libtensorflow_cc.so -DTENSORFLOW_INCLUDE_DIR=$TENSORFLOW_INCLUDE -DEIGEN_INCLUDE_DIR=$TENSORFLOW_INCLUDE/eigen -DTENSORFLOW_FWK_LIBRARY=$TENSORFLOW_LIBDIR/libtensorflow_framework.so
make -j ${CORES}
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
  <use name="protobuf-bw"/>
  <use name="grpc"/>
  <use name="eigen"/>
  <use name="tensorflow-cc"/>
  <use name="tensorflow-framework"/>
</tool>
EOF_TOOLFILE
sed -i 's~$INSTALLDIR~'$INSTALLDIR'~' tensorflow-serving.xml

mv tensorflow-serving.xml ${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/
scram setup tensorflow-serving

# setup batch submission (before conda to avoid python2/3 issues)
cd $CMSSW_BASE/src/SonicCMS/Brainwave/batch
python $CMSSW_BASE/src/Condor/Production/python/linkScripts.py

# setup for conda environment (kept separate from CMSSW)
cd $CMSSW_BASE/..

# setup miniconda
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh -b -p $CMSSW_BASE/../miniconda3
source $CMSSW_BASE/../miniconda3/etc/profile.d/conda.sh
conda activate

# setup aml env
conda create -y -n myamlenv Python=3.6 cython numpy
conda activate myamlenv
pip install --upgrade azureml-sdk[notebooks,automl,contrib] azureml-dataprep

# to get working version
AMLDIR=miniconda3/envs/myamlenv/lib/python3.6/site-packages
if [ -d $AMLDIR/tensorflow ]; then
	mv $AMLDIR/tensorflow $AMLDIR/tensorflow-bak
fi
ln -s $CMSSWTF $AMLDIR/tensorflow

# really terrible hack to make remote login work
sed -i 's/use_device_code=False/use_device_code=True/' $CMSSW_BASE/../miniconda3/envs/myamlenv/lib/python3.6/site-packages/azureml/_base_sdk_common/common.py

# remove the huge source code directory and intermediate products that are not needed to run
if [ -z "$DEBUG" ]; then
	cd $WORK
	rm -rf grpc inception_cmake
fi
