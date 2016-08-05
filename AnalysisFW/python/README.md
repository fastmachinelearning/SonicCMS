## Setup

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
    
4. Run the configuration files:

    ```
    cmsRun OpenDataTreeProducer_dataPAT_2011_cfg.py
    cmsRun OpenDataTreeProducer_mcPAT_2011_cfg.py
    ```

5. Open the tuples in ROOT:
 
    ```
    root OpenDataTree_*
    ```
