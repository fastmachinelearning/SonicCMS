# CMS Jet Image inference testing

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Updated instructions

For this branch, please use the following script to set up the work area and associated tools (`grpc`,`tensorflow-serving`):
```
wget https://raw.githubusercontent.com/kpedro88/2011-jet-inclusivecrosssection-ntupleproduction-optimized/kjp/1020_azureml/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_2_0/src
cmsenv
```

There is a separate script to connect to the AML server:
```
cd Jet2011
./setup_aml.sh -i -r
```
The script has several options (the separate options `-i` and `-r` allow installing only once, but then deploying multiple times):
* `-i`: install miniconda and aml-real-time-ai
* `-r`: run aml server setup script
* `-p [file]`: name of server params output json file (default=service_params.json)  

This script, if successful (and if executed with the `-r` option), will create a JSON file with the address and port for the remote server.

To get the various input files (the data file comes from [this file list](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)):
```
./download.sh
cd AnalysisFW/python
```

Now, the producer can run in either local or remote mode.
* Local mode:
```
cmsRun OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py
```
* Remote mode:
```
cmsRun OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py remote=1 params=$CMSSW_BASE/src/Jet2011/service_params.json
```

The remote mode timeout is set to 30 seconds by default. It can be changed e.g. to 10 seconds by adding the argument `timeout=10`.
If a server parameter JSON is not available, the server address and port can be specified using the arguments
`address=1.1.1.1` and `port=22`.
