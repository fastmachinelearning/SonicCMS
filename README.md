# SONIC (Services for Optimal Network Inference on Coprocessors) for CMS

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Updated instructions

For this branch, please use the following script to set up the work area and associated tools (`grpc`,`tensorflow-serving`,`miniconda`,`azureml`):
```
wget https://raw.githubusercontent.com/hls-fpga-machine-learning/SonicCMS/kjp/1020_azureml_ew/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_2_0/src
cmsenv
```

To get the various input files (the data file comes from [this file list](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)):
```
./download.sh
cd AnalysisFW/python
```

With the `miniconda` and `azureml` libraries installed, the `condapython3` executable can be used to run python scripts
that depend on them (to avoid confusion between the CMSSW and `miniconda` environments, a subshell is used).

The script `configure_aml.py` is used to create and register a model and start the associated service. The options of this script are:
* `-h`, `--help`: show this help message and exit
* `-p PARAMS`, `--params=PARAMS`: name of service & model params output json file (default = service_model_params.json)
* `-m MODEL`, `--model=MODEL`: use model with provided name (default = )
* `-r`, `--recreate`: recreate model (instead of use existing model)
* `-s SERVICE`, `--service=SERVICE`: use service with provided name (default = )
* `-d`, `--delete`: delete service and/or model (instead of starting)
* `-v`, `--verbose`: turn on informational printouts

To get started, using an existing model:
```
condapython3 configure_aml.py -m resnet50-model-kjp-float -s imagenet-infer
```
Follow the prompts on screen until the script is complete.

This script creates a JSON file with the address and port for the remote server, and the `model_id`.
(If you want to reuse the `model_id` in the JSON output file, simply omit the `-m MODEL` argument.)

Now, the producer can run in either local or remote mode.
* Local mode:
```
cmsRun jetImageTest_mc_cfg.py maxEvents=25
```
* Remote mode:
```
cmsRun jetImageTest_mc_cfg.py remote=1 params=$CMSSW_BASE/src/SonicCMS/AnalysisFW/python/service_model_params.json maxEvents=25
```

The remote mode timeout is set to 30 seconds by default. It can be changed e.g. to 10 seconds by adding the argument `timeout=10`.
If a server parameter JSON is not available, the server address and port can be specified using the arguments
`address=1.1.1.1` and `port=22`.

Batch job submission via HTCondor is supported, with the [CondorProduction](https://github.com/kpedro88/CondorProduction) package
used to handle job submission and management. Example commands:
```
cd $CMSSW_BASE/src/SonicCMS/AnalysisFW/batch
python $CMSSW_BASE/src/Condor/Production/python/cacheAll.py
python submitJobs.py -p -s -i /store/mc/RunIISpring18MiniAOD/BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph/MINIAODSIM/100X_upgrade2018_realistic_v10-v1/30000/24A0230C-B530-E811-ADE3-14187741120B.root -o root://cmseos.fnal.gov//store/user/pedrok/sonic -J test1N1 -N 1 -A "remote=1 params=service_model_params.json maxEvents=5000"
```

Once you are finished (if using remote mode), you can delete the service (and/or model):
```
condapython3 configure_aml.py -d -s quickstart-service
```
