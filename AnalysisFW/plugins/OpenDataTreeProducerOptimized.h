#ifndef OpenDataTreeProducerOptimized_h
#define OpenDataTreeProducerOptimized_h

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
#include "fastjet/contrib/Njettiness.hh"

using namespace edm;
using namespace reco;
using namespace std;
using namespace trigger;

class OpenDataTreeProducerOptimized : public edm::EDAnalyzer 
{
  public:

    explicit OpenDataTreeProducerOptimized(edm::ParameterSet const& cfg);
    virtual void beginJob();
    virtual void beginRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void analyze(edm::Event const& evt, edm::EventSetup const& iSetup);
    virtual void endRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void endJob();
    virtual ~OpenDataTreeProducerOptimized();
    
    enum MeasureDefinition_t {
        NormalizedMeasure=0,       // (beta,R0) 
        UnnormalizedMeasure,       // (beta) 
        GeometricMeasure,          // (beta) 
        NormalizedCutoffMeasure,   // (beta,R0,Rcutoff) 
        UnnormalizedCutoffMeasure, // (beta,Rcutoff) 
        GeometricCutoffMeasure,    // (beta,Rcutoff) 
	N_MEASURE_DEFINITIONS
    };
    enum AxesDefinition_t {
      KT_Axes=0,
      CA_Axes,
      AntiKT_Axes,   // (axAxesR0)
      WTA_KT_Axes,
      WTA_CA_Axes,
      Manual_Axes,
      OnePass_KT_Axes,
      OnePass_CA_Axes,
      OnePass_AntiKT_Axes,   // (axAxesR0)
      OnePass_WTA_KT_Axes,
      OnePass_WTA_CA_Axes,
      OnePass_Manual_Axes,
      MultiPass_Axes,
      N_AXES_DEFINITIONS
    };

    //float getTau(unsigned num, const edm::Ptr<reco::Jet> & object) const;
    float getTau(unsigned num, const reco::PFJet * object) const;


  private:
    
    // Measure definition : 
    unsigned                               measureDefinition_;
    double                                 beta_ ;
    double                                 R0_;
    double                                 Rcutoff_;

    // Axes definition : 
    unsigned                               axesDefinition_;
    int                                    nPass_;
    double                                 akAxesR0_;
    
    // Softdrop :    
    double                                 zCut_;
    
    std::auto_ptr<fastjet::contrib::Njettiness>   routine_; 

    // Function to help sort the jet wrt. pT
    static bool cmp_patjets(const pat::Jet &pj1, const pat::Jet &pj2) {
        return pj1.pt() > pj2.pt();
    }

    //---- Configurable parameters --------  
    bool            mIsMCarlo;
    bool            mUseGenInfo;
    bool            mPrintTriggerMenu;
    int             mMinNPFJets, mMaxRows;
    double          mMinPFPt, mMinGenPt, mMaxY, mMaxEta, mMinJJMass;
    int             mGoodVtxNdof;
    double          mGoodVtxZ; 
    edm::InputTag   mPFak5JetsName;
    edm::InputTag   mPFak7JetsName;

    
    // ---- PF Jet input tags ----- //
    edm::InputTag   mGenJetsName;
    edm::InputTag   mGenParticles;
    edm::InputTag   mSrcPFRho;
    edm::InputTag   mPFMET; 
    edm::InputTag   mOfflineVertices;
    
    //---- Trigger----------------------
    std::string                 processName_;
    std::vector<std::string>    triggerNames_;
    std::vector<unsigned int>   triggerIndex_;
    edm::InputTag               triggerResultsTag_;
    HLTConfigProvider           hltConfig_;

    // PF Candidates
    edm::InputTag pfCandidates_;
    
    // Output variables
    edm::Service<TFileService>  fs;
    TTree                       *mTree;

    
    //---- TTree variables --------
    
    static const UInt_t kMaxNjet = 64;
    static const UInt_t kMaxNtrg = 32;

    // PF jets
    /* UInt_t njet; */
    /* Float_t jet_pt[kMaxNjet]; */
    /* Float_t jet_eta[kMaxNjet]; */
    /* Float_t jet_phi[kMaxNjet]; */
    /* Float_t jet_E[kMaxNjet]; */
    /* Bool_t jet_tightID[kMaxNjet]; */
    /* Float_t jet_area[kMaxNjet]; */
    /* Float_t jet_jes[kMaxNjet]; */
    /* Int_t jet_igen[kMaxNjet]; */

    // PF jets
    UInt_t njet_ak7;
    Float_t jet_pt_ak7[kMaxNjet];
    Float_t jet_eta_ak7[kMaxNjet];
    Float_t jet_phi_ak7[kMaxNjet];
    Float_t jet_E_ak7[kMaxNjet];
    Float_t jet_msd_ak7[kMaxNjet];
    Float_t jet_area_ak7[kMaxNjet];
    Float_t jet_jes_ak7[kMaxNjet];    
    Float_t jet_tau21_ak7[kMaxNjet];
    Int_t jet_igen_ak7[kMaxNjet];
    Int_t jet_isW_ak7[kMaxNjet];
    Int_t jet_ncand_ak7[kMaxNjet];
    //Int_t ak7_to_ak5[kMaxNjet];

    // Jet composition
    /* Float_t chf[kMaxNjet]; */
    /* 	Float_t nhf[kMaxNjet]; */
    /* 	Float_t phf[kMaxNjet]; */
    /* 	Float_t elf[kMaxNjet]; */
    /* 	Float_t muf[kMaxNjet]; */
    /* 	Float_t hf_hf[kMaxNjet]; */
    /* 	Float_t hf_phf[kMaxNjet]; */
    /* 	Int_t hf_hm[kMaxNjet]; */
    /* 	Int_t hf_phm[kMaxNjet]; */
    /* 	Int_t chm[kMaxNjet]; */
    /* 	Int_t nhm[kMaxNjet]; */
    /* 	Int_t phm[kMaxNjet]; */
    /* 	Int_t elm[kMaxNjet]; */
    /* 	Int_t mum[kMaxNjet];    */
    /* Float_t beta[kMaxNjet];    */
    /* Float_t bstar[kMaxNjet]; */

    // Generated jets
    UInt_t ngen;
    Float_t gen_pt[kMaxNjet];
    Float_t gen_eta[kMaxNjet];
    Float_t gen_phi[kMaxNjet];
    Float_t gen_E[kMaxNjet];
    
    // Generated particles
    UInt_t ngenparticles;
    Float_t genparticle_pt[kMaxNjet];
    Float_t genparticle_eta[kMaxNjet];
    Float_t genparticle_phi[kMaxNjet];
    Float_t genparticle_E[kMaxNjet];
    Int_t genparticle_id[kMaxNjet];
    Int_t genparticle_dauId[kMaxNjet];
    Int_t genparticle_status[kMaxNjet];

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

    // Jet correction labels
    std::string mJetCorr_ak5;
    std::string mJetCorr_ak7;
    
    //PF Candidates
    std::vector<float> *etas, *phis, *pts;
    std::vector<int> *ids, *charges, *ak7indices;
    
    // c2numpy
    c2numpy_writer writer;
    
};

#endif
