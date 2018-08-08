# Instructions 

### get a data file and the model files

Download the model files, and the classifier outputs for resnet50

```
cd python
wget https://raw.githubusercontent.com/Lasagne/Recipes/master/examples/resnet50/imagenet_classes.txt
wget -O resnet50.pb "https://www.dropbox.com/s/8p783l38z8cp160/resnet50.pb?dl=0"
wget -O resnet50_classifier.pb "https://www.dropbox.com/s/mycy3dh154kfgtd/resnet50_classifier.pb?dl=0"
cd ..
```

Then `xrdcp` a file locally, [here's a list of files in DAS](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)

### compile
`scram b -j8`

### run
`cd python` <br>
`cmsRun OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py`
