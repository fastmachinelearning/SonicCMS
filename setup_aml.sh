#!/bin/bash

INSTALL=""
RUN=""
PFILE=service_params.json

usage(){
	EXIT=$1

	echo "setup_aml.sh [options]"
	echo ""
	echo "-i                  install miniconda and aml-real-time-ai"
	echo "-r                  run aml server setup script"
	echo "-p [file]           name of server params output json file (default=$PFILE)"
	echo "-h                  display this message and exit"

	exit $EXIT
}

# process options
while getopts "irp:h" opt; do
	case "$opt" in
	i) INSTALL=true
	;;
	r) RUN=true
	;;
	p) PFILE=optarg
	;;
	h) usage 0
	;;
	esac
done

if [ -z "$CMSSW_BASE" ]; then
	echo "Installation requires $CMSSW_BASE"
	exit 1
fi

if [ -n "$INSTALL" ]; then
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
fi

if [ -n "$RUN" ]; then
	cd $CMSSW_BASE/..
	
	# script to activate server
	cat << 'EOF_PYFILE' > setup_aml.py
import json
from amlrealtimeai import DeploymentClient

subscription_id = "80defacd-509e-410c-9812-6e52ed6a0016"
resource_group = "CMS_FPGA_Resources"
model_management_account = "CMS_FPGA_1"

model_name = "resnet50-model-nvt"
service_name = "quickstart-service"

deployment_client = DeploymentClient(subscription_id, resource_group, model_management_account)

service = deployment_client.get_service_by_name(service_name)

params = {"address": service.ipAddress, "port": service.port}
with open('service_params_tmp.json','w') as pfile:
    json.dump(params,pfile)

EOF_PYFILE

	# run script
	source $CMSSW_BASE/../miniconda3/etc/profile.d/conda.sh
	conda activate amlrealtimeai
	python3 setup_aml.py
	mv service_params_tmp.json $PFILE
fi
