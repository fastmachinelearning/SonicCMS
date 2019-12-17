# SONIC (Services for Optimized Network Inference on Coprocessors) for CMS

## Instructions

For this branch, please use the following script to set up the work area:
```
wget https://raw.githubusercontent.com/hls-fpga-machine-learning/SonicCMS/master/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_6_6/src
cmsenv
```

By default, this script installs only the Core package from this repository.
Each other package has its own dedicated script to install associated tools and dependencies,
which can be executed by the main setup script depending on which packages are selected using `-p`.
More packages can be added later (without rerunning the rest of the setup) by additionally supplying the `-i` option.
The setup script has additional options, which are listed here:
* `-f [fork]`: clone from specified fork (default = hls-fpga-machine-learning)
* `-b [branch]`: clone specified branch (default = master)
* `-c [version]`: use specified CMSSW version (default = CMSSW_10_6_6)
* `-p [packages]`: set up specified packages (allowed = TensorRT,Brainwave; or all)
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

## Running

Each package has its own `README.md` file with details about how to run its producers.
