#!/bin/bash

if [ -z "$CMSSW_BASE" ]; then
	echo "Installation requires $CMSSW_BASE"
	exit 1
fi

cd $CMSSW_BASE/..
CMSSWTF=$(dirname $(python3 -c "import tensorflow; print(tensorflow.__file__)"))

# setup miniconda
unset PYTHONPATH
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
