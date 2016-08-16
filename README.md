
## Description
CMSSW module producing flat tuples from 2011A Jet data.

Source code was forked from the SMPJ Analysis Framework:

https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW

https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Setup

Create project directories:
```

```

=======
# CMS Jet Tuple production 2011

This project is a CMSSW module producing flat tuples from 2011A Jet data.

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

The instruction assume that you will work on a VM properly contextualized for CMS, available from http://opendata.cern.ch/VM/CMS.

## Creating the working area

This step is only needed the first time you run this program:
```
mkdir WorkingArea
cd ./WorkingArea
cmsrel CMSSW_5_3_32
cd ./CMSSW_5_3_32/src
cmsenv
git cms-addpkg PhysicsTools/PatAlgos
git clone https://github.com/tamshai/cms-opendata-2011-jets/
cp cms-opendata-2011-jets/jetProducer_cfi.py PhysicsTools/PatAlgos/python/producersLayer1/
scram b
cd cms-opendata-2011-jets/AnalysisFW/python/

```

## Setting up additional files

With `cms-opendata-2011-jets/AnalysisFW/python/` as the current folder, run the following commands:

1. Download index files : 
    
    ```
    wget http://opendata.cern.ch/record/21/files/CMS_Run2011A_Jet_AOD_12Oct2013-v1_20000_file_index.txt
    wget http://opendata.cern.ch/record/1562/files/CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-80to120_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt 
    ```
    
2. Download JSON of good runs:

    ```
    wget http://opendata.cern.ch/record/1001/files/Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt
    ```
    
3. Make link to the condition databases:

    ```
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA FT_53_LV5_AN1 
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db FT_53_LV5_AN1_RUNA.db
    
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1 START53_LV6A1
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db START53_LV6A1.db
    ```
    
## Run the program:
To create tuples from data run the following command:

    cmsRun OpenDataTreeProducer_dataPAT_2011_cfg.py
    
This command creates tuples from Monte Carlo simulations:

    cmsRun OpenDataTreeProducer_mcPAT_2011_cfg.py
 
After running the code, you can browse the tuples by opening the produced files in ROOT:

    root OpenDataTree_*
 
Finally, run this command in the ROOT command prompt:

    TBrowser t
 
