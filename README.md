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
    ```
    cmsRun OpenDataTreeProducer_dataPAT_2011_cfg.py
    ```
This command creates tuples from Monte Carlo simulations:
    ```
    cmsRun OpenDataTreeProducer_mcPAT_2011_cfg.py
    ```

After running the code, you can browse the tuples by opening the produced files in ROOT:
 
    ```
    root OpenDataTree_*
    ```
Finally, run this command in the ROOT command prompt:
 
    ```
    TBrowser t
    ```


## Troubleshooting

### Missing condition database symlinks

```
----- Begin Fatal Exception 15-Jul-2016 13:49:27 CEST-----------------------
An exception of category 'StdException' occurred while
   [0] Constructing the EventProcessor
   [1] Constructing ESSource: class=PoolDBESSource label='GlobalTag'
Exception Message:
A std::exception was thrown.
Connection on "sqlite_file:./FT_53_LV5_AN1/AlCaRecoHLTpaths8e29_1e31_v13_offline.db" cannot be established ( CORAL : "ConnectionPool::getSessionFromNewConnection" from "CORAL/Services/ConnectionService" )
----- End Fatal Exception -------------------------------------------------
```

* Data:
```
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA FT_53_LV5_AN1 
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db FT_53_LV5_AN1_RUNA.db
```
* Monte Carlo:
```
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1 START53_LV6A1
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db START53_LV6A1.db
```

### Missing Global tag

```
----- Begin Fatal Exception 15-Jul-2016 14:29:18 CEST-----------------------
An exception of category 'Incomplete configuration' occurred while
   [0] Constructing the EventProcessor
   [1] Constructing ESSource: class=PoolDBESSource label='GlobalTag'
Exception Message:
Valid site-local-config not found at /cvmfs/cms.cern.ch/SITECONF/local/JobConfig/site-local-config.xml
----- End Fatal Exception -------------------------------------------------
```

Add the following lines to the Python configuration:

* Data:
```
    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db')
    process.GlobalTag.globaltag = 'FT_53_LV5_AN1::All'
```
* Monte Carlo:
```
    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db')    
    process.GlobalTag.globaltag = cms.string('START53_LV6A1::All')
```

### Missing environment

    bash: cmsRun: command not found

Source the environment by running:
    
    cmsenv

