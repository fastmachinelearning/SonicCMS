# CMS Jet Tuple production 2011 (performance optimisied)

This project is a CMSSW module producing flat tuples from 2011A Jet data.

Source code was originally forked from the SMPJ Analysis Framework: 
https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW  
https://github.com/cms-smpj/SMPJ/tree/v1.0/

The instruction assume that you will work on a VM properly contextualized for CMS, available from http://opendata.cern.ch/VM/CMS.

## Updated instructions

For this branch, please use the following script to set up the work area and associated tools (`grpc`,`tensorflow-serving`):
```
wget https://raw.githubusercontent.com/kpedro88/2011-jet-inclusivecrosssection-ntupleproduction-optimized/kjp/1020_azureml/setup.sh
chmod +x setup.sh
./setup.sh
cd CMSSW_10_2_0/src
cmsenv
```

There is a separate script to connect to the AML server:
```
cd Jet2011
./setup_aml.sh -i -r
```
The script has several options (the separate options `-i` and `-r` allow installing only once, but then deploying multiple times):
* `-i`: install miniconda and aml-real-time-ai
* `-r`: run aml server setup script
* `-p [file]`: name of server params output json file (default=service_params.json)  

## Creating the working area

This step is only needed the first time you run this program:
```bash
cmsrel CMSSW_5_3_32
cd CMSSW_5_3_32/src
cmsenv
git clone git@github.com:jmduarte/2011-jet-inclusivecrosssection-ntupleproduction-optimized.
git clone git@github.com:cms-externals/fastjet-contrib
cd fastjet-contrib
export FASTJET_BASE=`scramv1 tool tag fastjet FASTJET_BASE`
./configure --fastjet-config=$FASTJET_BASE/bin/fastjet-config --prefix=$PWD CXXFLAGS="-I$FASTJET_BASE/include -I$FASTJET_BASE/tools"
make
make check
make install
make fragile-shared
make fragile-shared-install
cd $CMSSW_BASE/src
cp  2011-jet-inclusivecrosssection-ntupleproduction-optimized/fastjet-contrib.xml $CMSSW_BASE/config/toolbox/$SCRAM_ARCH/tools/selected/fastjet-contrib.xml
scram setup fastjet-contrib
scram b
cd 2011-jet-inclusivecrosssection-ntupleproduction-optimized/AnalysisFW/python/
```

## Setting up additional files

With `cms-opendata-2011-jets-optimized/AnalysisFW/python/` as the current folder, run the following commands:

1. Download index files : 
    
    ```bash
    wget http://opendata.cern.ch/record/21/files/CMS_Run2011A_Jet_AOD_12Oct2013-v1_20000_file_index.txt
    wget http://opendata.cern.ch/record/1562/files/CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-80to120_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt 
	wget http://opendata.cern.ch/record/1611/files/CMS_MonteCarlo2011_Summer11LegDR_W1Jet_TuneZ2_7TeV-madgraph-tauola_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt
    ```
    
2. Download JSON of good runs:

    ```bash
    wget http://opendata.cern.ch/record/1001/files/Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt
    ```
    
3. Create links to the condition databases:

    ```bash
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA FT_53_LV5_AN1     
    ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1 START53_LV6A1
    ```
    
## Run the program:
To create tuples from data run the following command:

```bash
mkdir output
cmsRun OpenDataTreeProducerOptimized_dataPAT_2011_cfg.py
```
    
This command creates tuples from Monte Carlo simulations:

```bash
mkdir output
cmsRun OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py
```
 
After running the code, you can browse the tuples by opening the produced files in ROOT:

```bash
root OpenDataTree_*
```
 
Finally, run this command in the ROOT command prompt:

```cpp
TBrowser t;
```
 


## Tuple variables

* Properties of the event:

```cpp
    int             run;                // Run number
    float           lumi;               // Luminosity section
    long long       event;              // Event number
    float           ntrg;               // Number of triggers
    bool            triggers[ntrg];     // Trigger bits
    vector<string>  *triggernames;      // Trigger names
    float           prescales[ntrg];    // Trigger prescales
    float           met;                // Missing transverse energy
    float           sumet;              // Sum of transverse energy
    float           rho;                // Energy density
```


* Jets reconstructed using the anti-kT algorithm with a parameter R = 0.5 (short. AK5).

```cpp
    int     njet;           // Number of AK5 jets
    float   jet_pt[njet];   // Corrected transverse momentum
    float   jet_eta[njet];  // Pseudorapidity
    float   jet_phi[njet];  // Azimuthal angle
    float   jet_E[njet];    // Energy
```

* Other AK5 jet information

```cpp
    bool    jet_tightID[njet];  // Tight selection pass/fail
    float   jet_area[njet];     // Jet area in eta-phi plane
    float   jet_jes[njet];      // Jet energy correction
    int     jet_igen[njet];     // Index of the matching generated jet
```

* Composition values of the AK5 jets

```cpp
    float    chf[njet];      // Charged hadron energy fraction
    float    nhf[njet];      // Neutral hadron energy fraction
    float    phf[njet];      // Photon energy fraction
    float    elf[njet];      // Electron energy fraction
    float    muf[njet];      // Muon energy fraction
    float    hf_hf[njet];    // Forward calorimeter (HF) hadron energy fraction
    float    hf_phf[njet];   // HF photon energy fraction
    int      hf_hm[njet];    // HF hadron multiplicity
    int      hf_phm[njet];   // HF photon multiplicity
    int      chm[njet];      // Charged hadron multiplicity
    int      nhm[njet];      // Neutral hadron multiplicity
    int      phm[njet];      // Photon multiplicity
    int      elm[njet];      // Electron multiplicity
    int      mum[njet];      // Muon multiplicity
    float    beta[njet];     // Fraction of chf associated to the hard process
    float    bstar[njet];    // Fraction of chf associated to pile-up
```

* Jets reconstructed using the anti-kT algorithm with a parameter R = 0.7 (short. AK7)

```cpp
    int     njet_ak7;               // Number of jets
    float   jet_pt_ak7[njet_ak7];   // Transverse momentum
    float   jet_eta_ak7[njet_ak7];  // Pseudorapidity
    float   jet_phi_ak7[njet_ak7];  // Azimuthal angle
    float   jet_E_ak7[njet_ak7];    // Energy
    float   jet_area_ak7[njet_ak7]; // Jet area
    float   jet_jes_ak7[njet_ak7];  // Jet energy corection factor
    int     ak7_to_ak5[njet_ak7];   // Index of the corresponding AK5 jet 
```

* True properties of jets generated in the Monte Carlo simulation (only MC datasets)

```cpp
    int     ngen;           // Number of jets generated
    float   gen_pt[ngen];   // Transverse momentum
    float   gen_eta[ngen];  // Pseudorapidity
    float   gen_phi[ngen];  // Azimuthal angle
    float   gen_E[ngen];    // Energy

    float   pthat;          // Transverse momentum in the rest frame of the hard interaction
    float   mcweight;       // Monte Carlo weight of the event
```



## Preparing output for Machine Learning pipeline

When CMSSW runs over the first 500 events of the dataset, it produces three files in `output` named `params0.npy` through `params2.npy`. To make one file, increase the number of rows per file (currently 10000) in the `c2numpy_init` call, recompile, and rerun.

These files should match the ones in this repository.

You can load any of the files in a Python session. We can also convert this into pandas DataFrame and see how we can access the data

```python
>>> import numpy as np
>>> import pandas as pd
>>> params0 = np.load("output/params0.npy")
>>> params0.dtype.names
('run', 'lumi', 'event', 'njet_ak7', 'jet_pt_ak7', 'jet_eta_ak7', 'jet_phi_ak7', 'jet_E_ak7', 'jet_area_ak7', 'jet_jes_ak7', 'ak7pfcand_pt', 'ak7pfcand_eta', 'ak7pfcand_phi', 'ak7pfcand_id', 'ak7pfcand_charge', 'ak7pfcand_ijet')
>>> df = pd.DataFrame(params0)
>>> print df.iloc[:3]
      run  lumi     event  njet_ak7  jet_pt_ak7  jet_eta_ak7  jet_phi_ak7  \
0  160578   366  38174649         2   81.245902     1.601862    -0.247781   
1  160578   366  38174649         2   81.245902     1.601862    -0.247781   
2  160578   366  38174649         2   81.245902     1.601862    -0.247781   

    jet_E_ak7  jet_area_ak7  jet_jes_ak7  ak7pfcand_pt  ak7pfcand_eta  \
0  211.272537      1.515943     0.964843     22.249989       1.604204   
1  211.272537      1.515943     0.964843      7.718062       1.617311   
2  211.272537      1.515943     0.964843      6.549799       1.526909   

   ak7pfcand_phi  ak7pfcand_id  ak7pfcand_charge  ak7pfcand_ijet  
0      -0.197934            22                 0               0  
1      -0.116796           211                 1               0  
2      -0.263528          -211                -1               0  
```
