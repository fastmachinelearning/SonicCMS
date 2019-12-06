# SONIC (Services for Optimized Network Inference on Coprocessors) for CMS

## Updated instructions

For this branch, please use the following script to set up the work area:
```
wget https://raw.githubusercontent.com/hls-fpga-machine-learning/SonicCMS/abstract/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_6_6/src
cmsenv
```

By default, this script installs all packages (corresponding to non-`Core` directories in this repository).
Each package has its own dedicated script to install associated tools and dependencies, which is executed by the main setup script.
Instead, only desired packages can be selected for installation.
More packages can be added later (without rerunning the rest of the setup) using the `-i` option.
The setup script has additional options, which are listed here:
* `-f [fork]`: clone from specified fork (default = hls-fpga-machine-learning)
* `-b [branch]`: clone specified branch (default = abstract)
* `-c [version]`: use specified CMSSW version (default = CMSSW_10_6_6)
* `-p [packages]`: set up specified packages (allowed = TensorRT; or all)
* `-a [protocol]`: use protocol to clone (default = https, alternative = ssh)
* `-j [cores]`: run compilations on # cores (default = 1)
* `-d`: keep source code for debugging
* `-i`: install only (to add new packages, if setup was already run)
* `-h`: display help message and exit

To get the various input files (the data file comes from [this file list](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM)):
```
./download.sh
```
These files are stored in `Core` so they can be shared by the various packages.
