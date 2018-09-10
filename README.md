# CMS Jet Image inference testing

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Updated instructions

For this branch, please use the following script to set up the work area and associated tools (`grpc`,`tensorflow-serving`,`miniconda`,`aml-real-time-ai`):
```
wget https://raw.githubusercontent.com/kpedro88/2011-jet-inclusivecrosssection-ntupleproduction-optimized/kjp/1020_azureml/setup.sh
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

With the `miniconda` and `aml-real-time-ai` libraries installed, the `condapython3` executable can be used to run python scripts
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
condapython3 configure_aml.py -m resnet50-model-nvt -s quickstart-service
```
Follow the prompts on screen until the script is complete.

This script creates a JSON file with the address and port for the remote server, and the `model_id`.
(If you want to reuse the `model_id` in the JSON output file, simply omit the `-m MODEL` argument.)

Now, the producer can run in either local or remote mode.
* Local mode:
```
cmsRun jetImageTest_mc_cfg.py
```
* Remote mode:
```
cmsRun jetImageTest_mc_cfg.py remote=1 params=$CMSSW_BASE/src/Jet2011/AnalysisFW/python/service_model_params.json
```

The remote mode timeout is set to 30 seconds by default. It can be changed e.g. to 10 seconds by adding the argument `timeout=10`.
If a server parameter JSON is not available, the server address and port can be specified using the arguments
`address=1.1.1.1` and `port=22`.

Once you are finished (if using remote mode), you can delete the service (and/or model):
```
condapython3 configure_aml.py -d -s quickstart-service
```
