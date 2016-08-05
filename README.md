## Description
CMSSW module producing flat tuples from 2011A Jet data.

Source code was forked from the SMPJ Analysis Framework:

https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW

https://github.com/cms-smpj/SMPJ/tree/v1.0/

## Setup

Create project directories:
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

Data:

    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA FT_53_LV5_AN1 
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db FT_53_LV5_AN1_RUNA.db

Monte Carlo:

    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1 START53_LV6A1
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db START53_LV6A1.db


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

Add to the Python configuration:

Data:

    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db')
    process.GlobalTag.globaltag = 'FT_53_LV5_AN1::All'

Monte Carlo:

    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db')    
    process.GlobalTag.globaltag = cms.string('START53_LV6A1::All')


### Missing environment

    bash: cmsRun: command not found

Setup the environment by running:
    
    cmsenv

