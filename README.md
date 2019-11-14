# SONIC (Services for Optimal Network Inference on Coprocessors) for CMS

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Updated instructions

For this branch, please use the following script to set up the work area and associated tools (`tensorrt-server`):
```
wget https://raw.githubusercontent.com/hls-fpga-machine-learning/SonicCMS/pch/gpu/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_6_1_patch3/src
cmsenv
```

To get the various input files (the data file comes from [this file list](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)):
```
./download.sh
cd AnalysisFW/python
```

* Remote mode:
```
cmsRun jetImageTest_mc_cfg.py remote=1 address=ailab01.fnal.gov port=8001 maxEvents=25
```
(or the same for `HcalTest_mc_cfg.py`)

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
