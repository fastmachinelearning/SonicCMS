# Instructions 

### get a data file and the model files
`xrdcp` a file locally, [here's a list of files in DAS](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)
```
wget https://www.dropbox.com/s/8p783l38z8cp160/resnet50.pb?dl=0
wget https://www.dropbox.com/s/mycy3dh154kfgtd/resnet50_classifier.pb?dl=0
mv *.pb python/.
```

### compile
`scram b -j8`

### run
`cd python` <br>
`cmsRun OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py`
