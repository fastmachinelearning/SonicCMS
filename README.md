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
    
3. Create links to the condition databases:

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
 


## Tuple variables

* Properties of the event:

    UInt_t          run;                // Run number
    UInt_t          lumi;               // Luminosity section
    ULong64_t       event;              // Event number
    UInt_t          ntrg;               // Number of triggers
    Bool_t          triggers[ntrg];     // Trigger bits
    vector<string>  *triggernames;      // Trigger names
    UInt_t          prescales[ntrg];    // Trigger prescales
    Float_t         met;                // Missing transverse energy
    Float_t         sumet;              // Sum of transverse energy
    Float_t         rho;                // Energy density


* Jets reconstructed using the anti-kT algorithm with a parameter R = 0.5 (short. AK5). Number of jets and corrected four-momentum in cylindrical coordinates.

    UInt_t          njet;           // Number of AK5 jets
    Float_t         jet_pt[njet];   // Corrected transverse momentum
    Float_t         jet_eta[njet];  // Pseudorapidity
    Float_t         jet_phi[njet];  // Azimuthal angle
    Float_t         jet_E[njet];    // Energy

* Other AK5 jet information

    Bool_t          jet_tightID[njet];  // Tight selection pass/fail
    Float_t         jet_area[njet];     // Jet area in eta-phi plane
    Float_t         jet_jes[njet];      // Jet energy correction
    Int_t           jet_igen[njet];     // Index of the matching generated jet


* Composition values of the AK5 jets

    Float_t         chf[njet];      // Charged hadron energy fraction
    Float_t         nhf[njet];      // Neutral hadron energy fraction
    Float_t         phf[njet];      // Photon energy fraction
    Float_t         elf[njet];      // Electron energy fraction
    Float_t         muf[njet];      // Muon energy fraction
    Float_t         hf_hf[njet];    // Forward calorimeter (HF) hadron energy fraction
    Float_t         hf_phf[njet];   // HF photon energy fraction
    UInt_t          hf_hm[njet];    // HF hadron multiplicity
    UInt_t          hf_phm[njet];   // HF photon multiplicity
    UInt_t          chm[njet];      // Charged hadron multiplicity
    UInt_t          nhm[njet];      // Neutral hadron multiplicity
    UInt_t          phm[njet];      // Photon multiplicity
    UInt_t          elm[njet];      // Electron multiplicity
    UInt_t          mum[njet];      // Muon multiplicity
    Float_t         beta[njet];     // Fraction of chf associated to the hard process
    Float_t         bstar[njet];    // Fraction of chf associated to pile-up


* Jets reconstructed using the anti-kT algorithm with a parameter R = 0.7 (short. AK7)

    UInt_t          njet_ak7;               // Number of jets
    Float_t         jet_pt_ak7[njet_ak7];   // Transverse momentum
    Float_t         jet_eta_ak7[njet_ak7];  // Pseudorapidity
    Float_t         jet_phi_ak7[njet_ak7];  // Azimuthal angle
    Float_t         jet_E_ak7[njet_ak7];    // Energy
    Float_t         jet_area_ak7[njet_ak7]; // Jet area
    Float_t         jet_jes_ak7[njet_ak7];  // Jet energy corection factor
    Int_t           ak7_to_ak5[njet_ak7];   // Index of the corresponding AK5 jet 

* True quantities of the jet generated in the Monte Carlo simulation (only MC datasets)

    UInt_t          ngen;           // Number of jets generated
    Float_t         gen_pt[ngen];   // Transverse momentum
    Float_t         gen_eta[ngen];  // Pseudorapidity
    Float_t         gen_phi[ngen];  // Azimuthal angle
    Float_t         gen_E[ngen];    // Energy

    Float_t         pthat;          // Transverse momentum in the rest frame of the hard interaction
    Float_t         mcweight;       // Monte Carlo weight of the event
