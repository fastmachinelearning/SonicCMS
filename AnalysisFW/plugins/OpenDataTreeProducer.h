#ifndef OpenDataTreeProducer_h
#define OpenDataTreeProducer_h

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "JetMETCorrections/Objects/interface/JetCorrector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/JetCorrFactors.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

using namespace edm;
using namespace reco;
using namespace std;
using namespace trigger;

class OpenDataTreeProducer : public edm::EDAnalyzer 
{
  public:

    explicit OpenDataTreeProducer(edm::ParameterSet const& cfg);
    virtual void beginJob();
    virtual void beginRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void analyze(edm::Event const& evt, edm::EventSetup const& iSetup);
    virtual void endRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void endJob();
    virtual ~OpenDataTreeProducer();


  private:  

    // Function to help sort the jet wrt. pT
    static bool cmp_patjets(const pat::Jet &pj1, const pat::Jet &pj2) {
        return pj1.pt() > pj2.pt();
    }

    //---- Configurable parameters --------  
    bool            mIsMCarlo;
    bool            mUseGenInfo;
    bool            mPrintTriggerMenu;
    int             mMinNPFJets;
    double          mMinPFPt, mMinGenPt, mMaxY, mMinJJMass;
    int             mGoodVtxNdof;
    double          mGoodVtxZ; 
    edm::InputTag   mPFak5JetsName;
    edm::InputTag   mPFak7JetsName;

    
    // ---- PF Jet input tags ----- //
    edm::InputTag   mGenJetsName;
    edm::InputTag   mSrcPFRho;
    edm::InputTag   mPFMET; 
    edm::InputTag   mOfflineVertices;
    
    //---- Trigger----------------------
    std::string                 processName_;
    std::vector<std::string>    triggerNames_;
    std::vector<unsigned int>   triggerIndex_;
    edm::InputTag               triggerResultsTag_;
    HLTConfigProvider           hltConfig_;
    
    // Output variables
    edm::Service<TFileService>  fs;
    TTree                       *mTree;

    
    //---- TTree variables --------
    
    static const UInt_t kMaxNjet = 64;
    static const UInt_t kMaxNtrg = 32;

    // PF jets
    UInt_t ak5_njet;
    Float_t ak5_pt[kMaxNjet];
    Float_t ak5_eta[kMaxNjet];
    Float_t ak5_phi[kMaxNjet];
    Float_t ak5_E[kMaxNjet];
    Bool_t ak5_tightID[kMaxNjet];
    Float_t ak5_area[kMaxNjet];
    Float_t ak5_jes[kMaxNjet];
    Int_t ak5_igen[kMaxNjet];

    // PF jets
    UInt_t ak7_njet;
    Float_t ak7_pt[kMaxNjet];
    Float_t ak7_eta[kMaxNjet];
    Float_t ak7_phi[kMaxNjet];
    Float_t ak7_E[kMaxNjet];
    Bool_t ak7_tightID[kMaxNjet];
    Float_t ak7_area[kMaxNjet];
    Float_t ak7_jes[kMaxNjet];
    Int_t ak7_to_ak5[kMaxNjet];

    // Jet composition
    Float_t chf[kMaxNjet];
   	Float_t nhf[kMaxNjet];
   	Float_t phf[kMaxNjet];
   	Float_t elf[kMaxNjet];
   	Float_t muf[kMaxNjet];
   	Float_t hf_hf[kMaxNjet];
   	Float_t hf_phf[kMaxNjet];
   	Int_t hf_hm[kMaxNjet];
   	Int_t hf_phm[kMaxNjet];
   	Int_t chm[kMaxNjet];
   	Int_t nhm[kMaxNjet];
   	Int_t phm[kMaxNjet];
   	Int_t elm[kMaxNjet];
   	Int_t mum[kMaxNjet];   
    Float_t beta[kMaxNjet];   
    Float_t bstar[kMaxNjet];

    // Generated jets
    UInt_t ngen;
    Float_t gen_pt[kMaxNjet];
    Float_t gen_eta[kMaxNjet];
    Float_t gen_phi[kMaxNjet];
    Float_t gen_E[kMaxNjet];

    // Event identification
    UInt_t run;
    UInt_t lumi;
    ULong64_t event;

    // Triggers
    UInt_t ntrg;
    Bool_t triggers[kMaxNtrg];
    std::vector<std::string> triggernames;
    UInt_t prescales[kMaxNtrg];

    // MET, SuMET, rho, eventfilter
    Float_t met;
    Float_t sumet;
    Float_t rho;

    // MC variables
    Float_t pthat;
    Float_t mcweight;

};

#endif
