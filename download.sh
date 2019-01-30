#!/bin/bash

cd $CMSSW_BASE/src/SonicCMS/AnalysisFW/python
wget https://raw.githubusercontent.com/Lasagne/Recipes/master/examples/resnet50/imagenet_classes.txt
wget -O resnet50.pb "https://www.dropbox.com/s/8p783l38z8cp160/resnet50.pb?dl=0"
wget -O resnet50_classifier.pb "https://www.dropbox.com/s/mycy3dh154kfgtd/resnet50_classifier.pb?dl=0"
xrdcp root://cmsxrootd.fnal.gov//store/mc/RunIISpring18MiniAOD/BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph/MINIAODSIM/100X_upgrade2018_realistic_v10-v1/30000/24A0230C-B530-E811-ADE3-14187741120B.root data/store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root
