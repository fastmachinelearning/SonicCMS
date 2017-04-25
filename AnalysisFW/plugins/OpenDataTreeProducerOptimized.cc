

// Forked from SMPJ Analysis Framework
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW
// https://github.com/cms-smpj/SMPJ/tree/v1.0


#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <functional>
#include "TTree.h"
#include <vector>
#include <cassert>
#include <TLorentzVector.h>

// c2numpy convertion include
#include "2011-jet-inclusivecrosssection-ntupleproduction-optimized/AnalysisFW/interface/c2numpy.h"
#include "OpenDataTreeProducerOptimized.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/JetExtendedAssociation.h"
#include "DataFormats/JetReco/interface/JetID.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"

#include "RecoJets/JetAssociationProducers/src/JetTracksAssociatorAtVertex.h"

#include "fastjet/contrib/SoftDrop.hh"

OpenDataTreeProducerOptimized::OpenDataTreeProducerOptimized(edm::ParameterSet const &cfg) {
  mMinPFPt           = cfg.getParameter<double>                    ("minPFPt");
  mMinJJMass         = cfg.getParameter<double>                    ("minJJMass");
  mMaxY              = cfg.getParameter<double>                    ("maxY");
  mMaxEta            = cfg.getParameter<double>                    ("maxEta");
  mMinNPFJets        = cfg.getParameter<int>                       ("minNPFJets");
  mMaxRows           = cfg.getUntrackedParameter<int>              ("maxRows",10000);
  mPFak5JetsName     = cfg.getParameter<edm::InputTag>             ("pfak5jets");
  mPFak7JetsName     = cfg.getParameter<edm::InputTag>             ("pfak7jets");
  mOfflineVertices   = cfg.getParameter<edm::InputTag>             ("offlineVertices");
  mGoodVtxNdof       = cfg.getParameter<double>                    ("goodVtxNdof");
  mGoodVtxZ          = cfg.getParameter<double>                    ("goodVtxZ");
  mSrcPFRho          = cfg.getParameter<edm::InputTag>             ("srcPFRho");
  mPFMET             = cfg.getParameter<edm::InputTag>             ("pfmet");
  mGenJetsName       = cfg.getUntrackedParameter<edm::InputTag>    ("genjets",edm::InputTag(""));
  mGenParticles      = cfg.getUntrackedParameter<edm::InputTag>    ("genparticles",edm::InputTag(""));
  mPrintTriggerMenu  = cfg.getUntrackedParameter<bool>             ("printTriggerMenu",false);
  mIsMCarlo          = cfg.getUntrackedParameter<bool>             ("isMCarlo",false);
  mUseGenInfo        = cfg.getUntrackedParameter<bool>             ("useGenInfo",false);
  mMinGenPt          = cfg.getUntrackedParameter<double>           ("minGenPt",30);
  processName_       = cfg.getParameter<std::string>               ("processName");
  triggerNames_      = cfg.getParameter<std::vector<std::string> > ("triggerNames");
  triggerResultsTag_ = cfg.getParameter<edm::InputTag>             ("triggerResults");
  mJetCorr_ak5       = cfg.getParameter<std::string>               ("jetCorr_ak5");
  mJetCorr_ak7       = cfg.getParameter<std::string>               ("jetCorr_ak7");
  pfCandidates_ = cfg.getParameter<edm::InputTag>                  ("pfCandidates");


  
  measureDefinition_ = 0; //CMS default is normalized measure
  beta_ = 1.0; //CMS default is 1
  R0_ = 0.7; // CMS default is jet cone size
  Rcutoff_ = -999.0; // not used by default
  // variables for axes definition
  axesDefinition_ = 6; // CMS default is 1-pass KT axes
  nPass_ = -999; // not used by default
  akAxesR0_ = -999.0; // not used by default
  // for softdrop
  zCut_ = 0.1;
    
  // Get the measure definition
  fastjet::contrib::NormalizedMeasure          normalizedMeasure        (beta_,R0_);
  fastjet::contrib::UnnormalizedMeasure        unnormalizedMeasure      (beta_);
  fastjet::contrib::GeometricMeasure           geometricMeasure         (beta_);
  fastjet::contrib::NormalizedCutoffMeasure    normalizedCutoffMeasure  (beta_,R0_,Rcutoff_);
  fastjet::contrib::UnnormalizedCutoffMeasure  unnormalizedCutoffMeasure(beta_,Rcutoff_);
  fastjet::contrib::GeometricCutoffMeasure     geometricCutoffMeasure   (beta_,Rcutoff_);

  fastjet::contrib::MeasureDefinition const * measureDef = 0;
  switch ( measureDefinition_ ) {
  case UnnormalizedMeasure : measureDef = &unnormalizedMeasure; break;
  case GeometricMeasure    : measureDef = &geometricMeasure; break;
  case NormalizedCutoffMeasure : measureDef = &normalizedCutoffMeasure; break;
  case UnnormalizedCutoffMeasure : measureDef = &unnormalizedCutoffMeasure; break;
  case GeometricCutoffMeasure : measureDef = &geometricCutoffMeasure; break;
  case NormalizedMeasure : default : measureDef = &normalizedMeasure; break;
  } 

  // Get the axes definition
  fastjet::contrib::KT_Axes             kt_axes; 
  fastjet::contrib::CA_Axes             ca_axes; 
  fastjet::contrib::AntiKT_Axes         antikt_axes   (akAxesR0_);
  fastjet::contrib::WTA_KT_Axes         wta_kt_axes; 
  fastjet::contrib::WTA_CA_Axes         wta_ca_axes; 
  fastjet::contrib::OnePass_KT_Axes     onepass_kt_axes;
  fastjet::contrib::OnePass_CA_Axes     onepass_ca_axes;
  fastjet::contrib::OnePass_AntiKT_Axes onepass_antikt_axes   (akAxesR0_);
  fastjet::contrib::OnePass_WTA_KT_Axes onepass_wta_kt_axes;
  fastjet::contrib::OnePass_WTA_CA_Axes onepass_wta_ca_axes;
  fastjet::contrib::MultiPass_Axes      multipass_axes (nPass_);

  fastjet::contrib::AxesDefinition const * axesDef = 0;
  switch ( axesDefinition_ ) {
  case  KT_Axes : default : axesDef = &kt_axes; break;
  case  CA_Axes : axesDef = &ca_axes; break; 
  case  AntiKT_Axes : axesDef = &antikt_axes; break;
  case  WTA_KT_Axes : axesDef = &wta_kt_axes; break; 
  case  WTA_CA_Axes : axesDef = &wta_ca_axes; break; 
  case  OnePass_KT_Axes : axesDef = &onepass_kt_axes; break;
  case  OnePass_CA_Axes : axesDef = &onepass_ca_axes; break; 
  case  OnePass_AntiKT_Axes : axesDef = &onepass_antikt_axes; break;
  case  OnePass_WTA_KT_Axes : axesDef = &onepass_wta_kt_axes; break; 
  case  OnePass_WTA_CA_Axes : axesDef = &onepass_wta_ca_axes; break; 
  case  MultiPass_Axes : axesDef = &multipass_axes; break;
  };

  routine_ = std::auto_ptr<fastjet::contrib::Njettiness> ( new fastjet::contrib::Njettiness( *axesDef, *measureDef ) );
  
}

void OpenDataTreeProducerOptimized::beginJob() {
  
    etas   = new std::vector<float>; etas->clear();
    phis   = new std::vector<float>; phis->clear();
    pts    = new std::vector<float>; pts->clear();
    ids    = new std::vector<int>;   ids->clear();    
    charges= new std::vector<int>;   charges->clear();    
    ak7indices    = new std::vector<int>;   ak7indices->clear();
    
    mTree = fs->make< TTree >("OpenDataTree", "OpenDataTree");

    // Variables of the flat tuple
    // mTree->Branch("njet", &njet, "njet/i");
    // mTree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    // mTree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    // mTree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    // mTree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    // mTree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    // mTree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    // mTree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
    // mTree->Branch("jet_igen", jet_igen, "jet_igen[njet]/I");

    // AK7 variables
    mTree->Branch("njet_ak7", &njet_ak7, "njet_ak7/i");
    mTree->Branch("jet_pt_ak7", jet_pt_ak7, "jet_pt_ak7[njet_ak7]/F");
    mTree->Branch("jet_eta_ak7", jet_eta_ak7, "jet_eta_ak7[njet_ak7]/F");
    mTree->Branch("jet_phi_ak7", jet_phi_ak7, "jet_phi_ak7[njet_ak7]/F");
    mTree->Branch("jet_E_ak7", jet_E_ak7, "jet_E_ak7[njet_ak7]/F");
    mTree->Branch("jet_msd_ak7", jet_msd_ak7, "jet_msd_ak7[njet_ak7]/F");
    mTree->Branch("jet_area_ak7", jet_area_ak7, "jet_area_ak7[njet_ak7]/F");
    mTree->Branch("jet_jes_ak7", jet_jes_ak7, "jet_jes_ak7[njet_ak7]/F");
    mTree->Branch("jet_tau21_ak7", jet_tau21_ak7, "jet_tau21_ak7[njet_ak7]/F");
    mTree->Branch("jet_igen_ak7", jet_igen_ak7, "jet_igen_ak7[njet_ak7]/I");
    mTree->Branch("jet_isW_ak7", jet_isW_ak7, "jet_isW_ak7[njet_ak7]/I");
    mTree->Branch("jet_ncand_ak7", jet_ncand_ak7, "jet_ncand_ak7[njet_ak7]/I");
    //mTree->Branch("ak7_to_ak5", ak7_to_ak5, "ak7_to_ak5[njet_ak7]/I");

    mTree->Branch("ngen", &ngen, "ngen/i");
    mTree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
    mTree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
    mTree->Branch("gen_phi", gen_phi, "gen_phi[ngen]/F");
    mTree->Branch("gen_E", gen_E, "gen_E[ngen]/F");
    
    mTree->Branch("ngenparticles", &ngenparticles, "ngenparticles/i");
    mTree->Branch("genparticle_pt", genparticle_pt, "genparticle_pt[ngen]/F");
    mTree->Branch("genparticle_eta", genparticle_eta, "genparticle_eta[ngen]/F");
    mTree->Branch("genparticle_phi", genparticle_phi, "genparticle_phi[ngen]/F");
    mTree->Branch("genparticle_E", genparticle_E, "genparticle_E[ngen]/F");
    mTree->Branch("genparticle_id", genparticle_id, "genparticle_id[ngen]/I");
    mTree->Branch("genparticle_dauId1", genparticle_dauId1, "genparticle_dauId1[ngen]/I");
    mTree->Branch("genparticle_dauId2", genparticle_dauId2, "genparticle_dauId2[ngen]/I");
    mTree->Branch("genparticle_dauDR", genparticle_dauDR, "genparticle_dauDR[ngen]/F");
    mTree->Branch("genparticle_status", genparticle_status, "genparticle_status[ngen]/I");

    mTree->Branch("run", &run, "run/i");
    mTree->Branch("lumi", &lumi, "lumi/i");
    mTree->Branch("event", &event, "event/l");
    mTree->Branch("ntrg", &ntrg, "ntrg/i");
    mTree->Branch("triggers", triggers, "triggers[ntrg]/O");
    mTree->Branch("triggernames", &triggernames);
    mTree->Branch("prescales", prescales, "prescales[ntrg]/i");
    mTree->Branch("met", &met, "met/F");
    mTree->Branch("sumet", &sumet, "sumet/F");
    mTree->Branch("rho", &rho, "rho/F");
    mTree->Branch("pthat", &pthat, "pthat/F");
    mTree->Branch("mcweight", &mcweight, "mcweight/F");

    // mTree->Branch("chf", chf, "chf[njet]/F");   
    // mTree->Branch("nhf", nhf, "nhf[njet]/F");   
    // mTree->Branch("phf", phf, "phf[njet]/F");   
    // mTree->Branch("elf", elf, "elf[njet]/F");   
    // mTree->Branch("muf", muf, "muf[njet]/F");   
    // mTree->Branch("hf_hf", hf_hf, "hf_hf[njet]/F");   
    // mTree->Branch("hf_phf", hf_phf, "hf_phf[njet]/F");   
    // mTree->Branch("hf_hm", hf_hm, "hf_hm[njet]/i");    
    // mTree->Branch("hf_phm", hf_phm, "hf_phm[njet]/i");
    // mTree->Branch("chm", chm, "chm[njet]/i");   
    // mTree->Branch("nhm", nhm, "nhm[njet]/i");   
    // mTree->Branch("phm", phm, "phm[njet]/i");   
    // mTree->Branch("elm", elm, "elm[njet]/i");   
    // mTree->Branch("mum", mum, "mum[njet]/i");
    // mTree->Branch("beta", beta, "beta[njet]/F");   
    // mTree->Branch("bstar", bstar, "bstar[njet]/F");

    //PF Candidates
    mTree->Branch("ak7pfcand_pt", "std::vector<float>", &pts);
    mTree->Branch("ak7pfcand_eta", "std::vector<float>", &etas);
    mTree->Branch("ak7pfcand_phi", "std::vector<float>", &phis);
    mTree->Branch("ak7pfcand_id", "std::vector<int>", &ids);
    mTree->Branch("ak7pfcand_charge", "std::vector<int>", &charges);
    mTree->Branch("ak7pfcand_ijet", "std::vector<int>", &ak7indices);

    
    //c2numpy
   c2numpy_init(&writer, "output/params", mMaxRows);
   c2numpy_addcolumn(&writer, "run", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "lumi", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "event", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "njet_ak7", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "jet_pt_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_eta_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_phi_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_E_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_msd_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_area_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_jes_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_tau21_ak7", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "jet_isW_ak7", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "jet_ncand_ak7", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "ak7pfcand_pt", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "ak7pfcand_eta", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "ak7pfcand_phi", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "ak7pfcand_id", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "ak7pfcand_charge", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "ak7pfcand_ijet", C2NUMPY_INTC);

    
}

void OpenDataTreeProducerOptimized::endJob() {
  // c2numpy
  c2numpy_close(&writer);
}


void OpenDataTreeProducerOptimized::beginRun(edm::Run const &iRun,
                                     edm::EventSetup const &iSetup) {

    // Mapping trigger indices 
    bool changed(true);
    if (hltConfig_.init(iRun, iSetup, processName_, changed) && changed) {

        // List of trigger names and indices 
        // are not emptied between events, must be done here
        triggerIndex_.clear();
        triggernames.clear();

        // Iterate over all active triggers of the AOD file
        auto name_list = hltConfig_.triggerNames();
        for (std::string name_to_search: triggerNames_) {

            // Find the version of jet trigger that is active in this run 
            for (std::string name_candidate: name_list) {

                // Match the prefix to the full name (eg. HLT_Jet30 to HLT_Jet30_v10)
                if ( name_candidate.find(name_to_search + "_v") != std::string::npos ) {
                    // Save index corresponding to the trigger
                    triggerIndex_.push_back(hltConfig_.triggerIndex(name_candidate));

                    // Save the trigger name
                    triggernames.push_back("jt" + name_to_search.substr(7, string::npos));
                    break;            
                }
            }
        }
    }

    // Retrieve cross section of the simulated process
    mcweight = 0;
    if (mIsMCarlo) {

        edm::Handle<GenRunInfoProduct> genRunInfo;
        iRun.getByLabel("generator", genRunInfo );

        // Save only the cross section, since the total number of 
        // generated events is not available in this context (!!)
        mcweight = genRunInfo->crossSection();
        std::cout << "Cross section: " <<  mcweight << std::endl;
    }
    
}


void OpenDataTreeProducerOptimized::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {
  
    etas->clear();
    phis->clear();
    pts->clear();
    ids->clear();    
    charges->clear();    
    ak7indices->clear();

    // Event info
    run = event_obj.id().run();
    lumi = event_obj.luminosityBlock();
    event = event_obj.id().event();

    // Triggers
    edm::Handle<edm::TriggerResults>   triggerResultsHandle_;
    event_obj.getByLabel(triggerResultsTag_, triggerResultsHandle_);

    // Sanity checks
    assert(triggerResultsHandle_.isValid() && "Error in getting TriggerResults from Event!");
    assert(triggerResultsHandle_->size() == hltConfig_.size() && "Size mismatch between triggerResultsHandle_ and hltConfig_");
    
    // Number of triggers to be saved
    ntrg = triggerIndex_.size();

    // Iterate only over the selected jet triggers
    for (unsigned itrig = 0; itrig < ntrg; itrig++) {

        // Trigger bit
        Bool_t isAccepted = triggerResultsHandle_->accept(triggerIndex_[itrig]);
        triggers[itrig] = isAccepted;

        // Trigger prescales are retrieved using the trigger name
        std::string trgName = hltConfig_.triggerName(triggerIndex_[itrig]);
	const std::pair< int, int > prescalePair(hltConfig_.prescaleValues(event_obj, iSetup, trgName));

        // Total prescale: PreL1*PreHLT 
        prescales[itrig] = prescalePair.first*prescalePair.second;   
    }    

    // Rho
    Handle< double > rho_handle;
    event_obj.getByLabel(mSrcPFRho, rho_handle);
    rho = *rho_handle;


    // Generator Info

    // Retrieve pthat and mcweight (only MC)
    pthat = 0;
    if (mIsMCarlo && mUseGenInfo) {
        Handle< GenEventInfoProduct > hEventInfo;
        event_obj.getByLabel("generator", hEventInfo);

        // Monte Carlo weight (NOT AVAILABLE FOR 2011 MC!!)
        //mcweight = hEventInfo->weight();
        
        // Pthat 
        if (hEventInfo->hasBinningValues()) {
            pthat = hEventInfo->binningValues()[0];
        }
    }

    // Generator-level jets and particles
    ngen = 0;
    ngenparticles = 0;
    if (mIsMCarlo) {

        Handle< GenJetCollection > genjets;
        event_obj.getByLabel(mGenJetsName, genjets);
    
        // Index of the simulated jet
        int gen_index = 0; 

        for (GenJetCollection::const_iterator i_gen = genjets->begin(); i_gen != genjets->end(); i_gen++)  {

            // pT and rapidity selection
            if (i_gen->pt() > mMinGenPt && fabs(i_gen->y()) < mMaxY && fabs(i_gen->eta()) < mMaxEta) {
                gen_pt[gen_index] = i_gen->pt();
                gen_eta[gen_index] = i_gen->eta();
                gen_phi[gen_index] = i_gen->phi();
                gen_E[gen_index] = i_gen->energy();
                gen_index++;
            }
        }

        // Number of generated jets in this event
        ngen = gen_index;
	
        Handle< GenParticleCollection > genparticles;
        event_obj.getByLabel(mGenParticles, genparticles);
    
        // Index of the simulated jet
        gen_index = 0; 

        for (GenParticleCollection::const_iterator i_gen = genparticles->begin(); i_gen != genparticles->end(); i_gen++)  {

            // pT, rapidity selection
            if (i_gen->pt() > mMinGenPt && fabs(i_gen->y()) < mMaxY && fabs(i_gen->eta()) < mMaxEta) {
	      //pdg id selection; for now save only W
	      //if (abs(i_gen->pdgId()) == 24) {
	      genparticle_pt[gen_index] = i_gen->pt();
	      genparticle_eta[gen_index] = i_gen->eta();
	      genparticle_phi[gen_index] = i_gen->phi();
	      genparticle_E[gen_index] = i_gen->energy();
	      genparticle_id[gen_index] = i_gen->pdgId();
	      genparticle_status[gen_index] = i_gen->status();
	      genparticle_dauId1[gen_index] = -1;
	      genparticle_dauId2[gen_index] = -1;
	      genparticle_dauDR[gen_index] = -1;
	      if (abs(i_gen->pdgId()) == 24) {		
		//std::cout << i_gen->pdgId() << std::endl;
		unsigned n = i_gen->numberOfDaughters();
		if (n>=2) {		  
		  const Candidate * d1 = i_gen->daughter( 0 );
		  const Candidate * d2 = i_gen->daughter( 1 );
		  int dauId1 = d1->pdgId();	  
		  int dauId2 = d2->pdgId();	  
		  genparticle_dauId1[gen_index] = dauId1;
		  genparticle_dauId2[gen_index] = dauId2;
		  genparticle_dauDR[gen_index] = reco::deltaR( d1->eta(),
							       d1->phi(),
							       d2->eta(),
							       d2->phi());
		}
		// for(unsigned j = 0; j < n; ++ j) {
		//   const Candidate * d = i_gen->daughter( j );
		//   int dauId = d->pdgId();	  
		//   genparticle_dauId1[gen_index] = dauId;
		// }
	      }	      
	      gen_index++;
	    }
	}	
        // Number of generated particles in this event
        ngenparticles = gen_index;
    }

    // Vertex Info
    Handle<reco::VertexCollection> recVtxs;
    event_obj.getByLabel(mOfflineVertices, recVtxs);


    // PF AK5 Jets

    // edm::Handle<reco::PFJetCollection> ak5_handle;
    // event_obj.getByLabel(mPFak5JetsName, ak5_handle);
    // const JetCorrector* corrector_ak5 = JetCorrector::getJetCorrector(mJetCorr_ak5, iSetup);

    // // Jet Track Association (JTA)
    // edm::Handle <reco::TrackCollection> tracks_h;
    // event_obj.getByLabel ("generalTracks", tracks_h);
    // std::auto_ptr<reco::JetTracksAssociation::Container> tracksInJets (new reco::JetTracksAssociation::Container (reco::JetRefBaseProd(ak5_handle)));
    // // format inputs
    // std::vector <edm::RefToBase<reco::Jet> > allJets;
    // allJets.reserve (ak5_handle->size());
    // for (unsigned i = 0; i < ak5_handle->size(); ++i)
    // {
    //   edm::RefToBase<reco::Jet> jetRef(edm::Ref<reco::PFJetCollection>(ak5_handle, i));
    //   allJets.push_back(jetRef);
    // }
    // std::vector <reco::TrackRef> allTracks;
    // allTracks.reserve(tracks_h->size());
    // for (unsigned i = 0; i < tracks_h->size(); ++i) 
    //   allTracks.push_back (reco::TrackRef(tracks_h, i));
    // // run JTA algorithm
    // JetTracksAssociationDRVertex mAssociator(0.5); // passed argument: 0.5 cone size
    // mAssociator.produce (&*tracksInJets, allJets, allTracks);
  
    // // Index of the selected jet 
    // int ak5_index = 0;

    // Jet energy correction factor
    double jec = -1.0;

    // Jets will be unsorted in pT after applying JEC,  
    // therefore store corrected jets in a new collection (map): key (double) is pT * -1 (key), 
    // value (std::pair<PFJet*, double>) is pair of original jet iterator and corresponding JEC factor
    std::map<double, std::pair<reco::PFJetCollection::const_iterator, double> > sortedJets;
    // for (auto i_ak5jet_orig = ak5_handle->begin(); i_ak5jet_orig != ak5_handle->end(); ++i_ak5jet_orig) {
    //     // take jet energy correction and get corrected pT
    //     jec = corrector_ak5->correction(*i_ak5jet_orig, event_obj, iSetup);
    //     // Multiply pT by -1 in order to have largest pT jet first (sorted in ascending order by default)
    //     sortedJets.insert(std::pair<double, std::pair<reco::PFJetCollection::const_iterator, double> >(-1 * i_ak5jet_orig->pt() * jec, std::pair<reco::PFJetCollection::const_iterator, double>(i_ak5jet_orig, jec)));
    // }

    // // Iterate over the jets (sorted in pT) of the event
    // for (auto i_ak5jet_orig = sortedJets.begin(); i_ak5jet_orig != sortedJets.end(); ++i_ak5jet_orig) {

    //     // Apply jet energy correction "on the fly":
    //     // copy original (uncorrected) jet;
    //     PFJet corjet = *((i_ak5jet_orig->second).first);
    //     // take stored JEC factor
    //     jec = (i_ak5jet_orig->second).second;
    //     // apply JEC
    //     corjet.scaleEnergy(jec);
    //     // pointer for further use
    //     const PFJet* i_ak5jet = &corjet;

    //     // Skip the current iteration if jet is not selected
    //     if (fabs(i_ak5jet->y()) > mMaxY || 
    //         (i_ak5jet->pt()) < mMinPFPt ||
    // 	    fabs(i_ak5jet->eta()) > mMaxEta ) {
    //         continue;
    //     }

    //     // Computing beta and beta*

    //     // Get tracks
    //     reco::TrackRefVector tracks = reco::JetTracksAssociation::getValue(*tracksInJets, *((i_ak5jet_orig->second).first));

    //     float sumTrkPt(0.0), sumTrkPtBeta(0.0),sumTrkPtBetaStar(0.0);
    //     beta[ak5_index] = 0.0;
    //     bstar[ak5_index] = 0.0;
        
    //     // Loop over tracks of the jet
    //     for(auto i_trk = tracks.begin(); i_trk != tracks.end(); i_trk++) {

    //         if (recVtxs->size() == 0) break;
            
    //         // Sum pT
    //         sumTrkPt += (*i_trk)->pt();
            
    //         // Loop over vertices
    //         for (unsigned ivtx = 0; ivtx < recVtxs->size(); ivtx++) {
    //             reco::Vertex vertex = (*recVtxs)[ivtx];

    //             // Loop over tracks associated with the vertex
    //             bool flagBreak = false;
    //             if (!(vertex.isFake()) && 
    //                 vertex.ndof() >= mGoodVtxNdof && 
    //                 fabs(vertex.z()) <= mGoodVtxZ) {
                    
    //                 for(auto i_vtxTrk = vertex.tracks_begin(); i_vtxTrk != vertex.tracks_end(); ++i_vtxTrk) {
                        
    //                     // Match the jet track to the track from the vertex
    //                     reco::TrackRef trkRef(i_vtxTrk->castTo<reco::TrackRef>());
                        
    //                     // Check for matching vertices
    //                     if (trkRef == (*i_trk)) {
    //                         if (ivtx == 0) {
    //                             sumTrkPtBeta += (*i_trk)->pt();
    //                         }
    //                         else {
    //                             sumTrkPtBetaStar += (*i_trk)->pt();
    //                         }
    //                         flagBreak = true;
    //                         break;
    //                     } 
    //                 } 
    //                 if(flagBreak)
    //                   break;
    //             } 
    //         } 
    //     }
    //     if (sumTrkPt > 0) {
    //         beta[ak5_index]   = sumTrkPtBeta/sumTrkPt;
    //         bstar[ak5_index]  = sumTrkPtBetaStar/sumTrkPt;
    //     } 


    //     // Jet composition
    //     // (all energy fractions have to be multiplied by the JEC factor)
    //     chf[ak5_index]     = i_ak5jet->chargedHadronEnergyFraction() * jec;
    //     nhf[ak5_index]     = (i_ak5jet->neutralHadronEnergyFraction() + i_ak5jet->HFHadronEnergyFraction()) * jec;
    //     phf[ak5_index]     = i_ak5jet->photonEnergyFraction() * jec;
    //     elf[ak5_index]     = i_ak5jet->electronEnergyFraction() * jec;
    //     muf[ak5_index]     = i_ak5jet->muonEnergyFraction() * jec;
    //     hf_hf[ak5_index]   = i_ak5jet->HFHadronEnergyFraction() * jec;
    //     hf_phf[ak5_index]  = i_ak5jet->HFEMEnergyFraction() * jec;
    //     hf_hm[ak5_index]   = i_ak5jet->HFHadronMultiplicity();
    //     hf_phm[ak5_index]  = i_ak5jet->HFEMMultiplicity();
    //     chm[ak5_index]     = i_ak5jet->chargedHadronMultiplicity();
    //     nhm[ak5_index]     = i_ak5jet->neutralHadronMultiplicity();
    //     phm[ak5_index]     = i_ak5jet->photonMultiplicity();
    //     elm[ak5_index]     = i_ak5jet->electronMultiplicity();
    //     mum[ak5_index]     = i_ak5jet->muonMultiplicity();
        
    //     int npr      = i_ak5jet->chargedMultiplicity() + i_ak5jet->neutralMultiplicity();

    //     bool isHighEta = fabs(i_ak5jet->eta()) > 2.4;
    //     bool isLowEta = fabs(i_ak5jet->eta()) <= 2.4 && 
    //                     nhf[ak5_index] < 0.9 &&
    //                     phf[ak5_index] < 0.9 && 
    //                     elf[ak5_index] < 0.99 && 
    //                     chf[ak5_index] > 0 && 
    //                     chm[ak5_index] > 0;
    //     bool tightID =  npr > 1 && 
    //                     phf[ak5_index] < 0.99 && 
    //                     nhf[ak5_index] < 0.99 &&
    //                     (isLowEta || isHighEta);


    //     // Variables of the tuple
    //     jet_tightID[ak5_index] = tightID;
    //     jet_area[ak5_index] = i_ak5jet->jetArea();
    //     jet_jes[ak5_index] = jec; // JEC factor

    //     // p4 is already corrected!
    //     auto p4 = i_ak5jet->p4();
    //     jet_pt[ak5_index]   = p4.Pt();
    //     jet_eta[ak5_index]  = p4.Eta();
    //     jet_phi[ak5_index]  = p4.Phi();
    //     jet_E[ak5_index]    = p4.E(); 
        
    //     // Matching a GenJet to this PFjet
    //     jet_igen[ak5_index] = 0;
    //     if (mIsMCarlo && ngen > 0) {

    //         // Index of the generated jet matching this PFjet
    //         jet_igen[ak5_index] = -1; // is -1 if no matching jet

    //         // Search generated jet with minimum distance to this PFjet   
    //         float r2min(999);
    //         for (unsigned int gen_index = 0; gen_index != ngen; gen_index++) {
    //             double deltaR2 = reco::deltaR2( jet_eta[ak5_index], 
    //                                             jet_phi[ak5_index],
    //                                             gen_eta[gen_index], 
    //                                             gen_phi[gen_index]);
    //             if (deltaR2 < r2min) {
    //                 r2min = deltaR2;
    //                 jet_igen[ak5_index] = gen_index;
    //             }
    //         }
    //     }
        
    // ak5_index++;
    // }  
    // // Number of selected jets in the event
    // njet = ak5_index;    


    // Four leading AK7 Jets

    edm::Handle<reco::PFJetCollection> ak7_handle;
    event_obj.getByLabel(mPFak7JetsName, ak7_handle);
    const JetCorrector* corrector_ak7 = JetCorrector::getJetCorrector(mJetCorr_ak7, iSetup);

    // Index of the selected jet 
    int ak7_index = 0;

    // Jets will be unsorted in pT after applying JEC,  
    // therefore store corrected jets in a new collection (map): key (double) is pT * -1 (key), 
    // value (std::pair<PFJet*, double>) is pair of original jet iterator and corresponding JEC factor
    sortedJets.clear();
    for (auto i_ak7jet_orig = ak7_handle->begin(); i_ak7jet_orig != ak7_handle->end(); ++i_ak7jet_orig) {
        // take jet energy correction and get corrected pT
        jec = corrector_ak7->correction(*i_ak7jet_orig, event_obj, iSetup);
        // Multiply pT by -1 in order to have largest pT jet first (sorted in ascending order by default)
        sortedJets.insert(std::pair<double, std::pair<reco::PFJetCollection::const_iterator, double> >(-1 * i_ak7jet_orig->pt() * jec, std::pair<reco::PFJetCollection::const_iterator, double>(i_ak7jet_orig, jec)));
    }

    // Iterate over the jets (sorted in pT) of the event
    njet_ak7 = 0;
    for (auto i_ak7jet_orig = sortedJets.begin(); i_ak7jet_orig != sortedJets.end(); ++i_ak7jet_orig) {

        // Apply jet energy correction "on the fly":
        // copy original (uncorrected) jet;
        PFJet corjet = *((i_ak7jet_orig->second).first);
        // take stored JEC factor
        jec = (i_ak7jet_orig->second).second;
        // apply JEC
        corjet.scaleEnergy(jec);
        // pointer for further use
        const PFJet* i_ak7jet = &corjet;

        // Skip the current iteration if jet is not selected
        if (fabs(i_ak7jet->y()) > mMaxY || 
            (i_ak7jet->pt()) < mMinPFPt ||
	    fabs(i_ak7jet->eta()) > mMaxEta) {
            continue;
        }
	njet_ak7++;
    }
    for (auto i_ak7jet_orig = sortedJets.begin(); i_ak7jet_orig != sortedJets.end(); ++i_ak7jet_orig) {

        // Apply jet energy correction "on the fly":
        // copy original (uncorrected) jet;
        PFJet corjet = *((i_ak7jet_orig->second).first);
        // take stored JEC factor
        jec = (i_ak7jet_orig->second).second;
        // apply JEC
        corjet.scaleEnergy(jec);
        // pointer for further use
        const PFJet* i_ak7jet = &corjet;

        // Skip the current iteration if jet is not selected
        if (fabs(i_ak7jet->y()) > mMaxY || 
            (i_ak7jet->pt()) < mMinPFPt ||
	    fabs(i_ak7jet->eta()) > mMaxEta) {
            continue;
        }

	
	//edm::Ptr<reco::Jet> jetPtr =  i_ak7jet->masterClone().castTo<edm::Ptr<reco::Jet> >();
	//float tau1 = getTau( 1, jetPtr );
	//float tau2 = getTau( 2, jetPtr );
	float tau1 = getTau( 1, i_ak7jet );
	float tau2 = getTau( 2, i_ak7jet );
	
        jet_tau21_ak7[ak7_index] = tau2/tau1;
	                                                                                                                 
	std::vector<fastjet::PseudoJet>   lClusterParticles;	
	for ( unsigned ida = 0; ida < i_ak7jet->numberOfDaughters(); ++ida ) 
	  {
	    reco::Candidate const * cand = i_ak7jet->daughter(ida);	                                                                                                
	    fastjet::PseudoJet pPart(cand->px(),cand->py(),cand->pz(),cand->energy());                                                                                                      
	    lClusterParticles.emplace_back(pPart);                                             
	  }
	//std::sort(lClusterParticles.begin(),lClusterParticles.end(),JetTools::orderPseudoJet);
	fastjet::JetDefinition ak7Jet_def(fastjet::antikt_algorithm, R0_);
	fastjet::ClusterSequence lCClust_seq(fastjet::sorted_by_pt(lClusterParticles), ak7Jet_def);
	//std::vector<fastjet::PseudoJet> tempJets = fastjet::sorted_by_pt(fjClusterSeq_->inclusive_jets(jetPtMin_));
	std::vector<fastjet::PseudoJet> inclusive_jets = fastjet::sorted_by_pt(lCClust_seq.inclusive_jets(0));
	fastjet::contrib::SoftDrop sd(beta_, zCut_, R0_ );
	fastjet::PseudoJet sd_jet = sd(inclusive_jets[0]);
	
        // Variables of the tuple
        jet_area_ak7[ak7_index] = i_ak7jet->jetArea();
        jet_jes_ak7[ak7_index] = jec; // JEC factor

        // p4 is already corrected!
        auto p4 = i_ak7jet->p4();
        jet_pt_ak7[ak7_index]   = p4.Pt();
        jet_eta_ak7[ak7_index]  = p4.Eta();
        jet_phi_ak7[ak7_index]  = p4.Phi();
        jet_E_ak7[ak7_index]    = p4.E();
	jet_msd_ak7[ak7_index]  = sd_jet.m();
        
        // Matching AK5 jet to this AK7 jet
        // Index of the generated jet matching this PFjet
        //ak7_to_ak5[ak7_index] = -1; // -1 if no matching jet

        // float r2min(999);
        // for (unsigned int ak5_index = 0; ak5_index != njet; ak5_index++) {

        //     // Compute distance squared
        //     double deltaR2 = reco::deltaR2( jet_eta_ak7[ak7_index], 
        //                                     jet_phi_ak7[ak7_index],
        //                                     jet_eta[ak5_index], 
        //                                     jet_phi[ak5_index]);
        //     if (deltaR2 < r2min) {
        //         r2min = deltaR2;
        //         ak7_to_ak5[ak7_index] = ak5_index;
        //     }
        // }

	
        // Matching a GenParticle to this PFjet
        jet_igen_ak7[ak7_index] = -1; // is -1 if no matching W within dR < 0.7
        jet_isW_ak7[ak7_index] = 0; // isW is 0 if no matching W within dR < 0.7
        if (mIsMCarlo && ngenparticles > 0) {

            // Search for generated W with minimum distance to this PFjet   
            float r2min(999);
            for (unsigned int gen_index = 0; gen_index != ngen; gen_index++) {
	      if (genparticle_id[gen_index]==24) {
                double deltaR2 = reco::deltaR2( jet_eta_ak7[ak7_index], 
                                                jet_phi_ak7[ak7_index],
                                                genparticle_eta[gen_index], 
                                                genparticle_phi[gen_index]);
                if (deltaR2 < r2min && deltaR2 < R0_*R0_) {
		  r2min = deltaR2;
		  jet_igen_ak7[ak7_index] = gen_index;
                }
	      }
            }
	    jet_isW_ak7[ak7_index] = (abs(genparticle_id[jet_igen_ak7[ak7_index]])==24
				      && abs(genparticle_dauId1[jet_igen_ak7[ak7_index]])<6
				      && abs(genparticle_dauId2[jet_igen_ak7[ak7_index]])<6				      
				      && abs(genparticle_dauDR[jet_igen_ak7[ak7_index]])>0
				      && abs(genparticle_dauDR[jet_igen_ak7[ak7_index]])<R0_);
        }
	
        jet_ncand_ak7[ak7_index] = i_ak7jet->numberOfDaughters();

	// PF Candidates in AK7 jet
	int charge = 0;
	for ( unsigned ida = 0; ida < i_ak7jet->numberOfDaughters(); ++ida ) 
	  {
	    reco::Candidate const * cand = i_ak7jet->daughter(ida);
	    pts->push_back(cand->pt());
	    etas->push_back(cand->eta());
	    phis->push_back(cand->phi());
	    ids->push_back(cand->pdgId());
	    if (cand->pdgId() == 22 || cand->pdgId() == 130 || cand->pdgId() == 0 || cand->pdgId() == 1 || cand->pdgId() == 2) {
	      charge = 0;
	    }
	    else {	      
	      charge = cand->pdgId()/abs(cand->pdgId());
	    }
	    charges->push_back(charge);
	    ak7indices->push_back(ak7_index);

	    // c2numpy	    
	    c2numpy_intc(&writer, run);
	    c2numpy_intc(&writer, lumi);
	    c2numpy_intc(&writer, event);
	    c2numpy_intc(&writer, njet_ak7);
	    c2numpy_float64(&writer, p4.Pt());
	    c2numpy_float64(&writer, p4.Eta());
	    c2numpy_float64(&writer, p4.Phi());
	    c2numpy_float64(&writer, p4.E());
	    c2numpy_float64(&writer, sd_jet.m());
	    c2numpy_float64(&writer, i_ak7jet->jetArea());
	    c2numpy_float64(&writer, jec);	    
	    c2numpy_float64(&writer, jet_tau21_ak7[ak7_index]);
	    c2numpy_intc(&writer, jet_isW_ak7[ak7_index]);
	    c2numpy_intc(&writer, jet_ncand_ak7[ak7_index]);
	    c2numpy_float64(&writer, cand->pt());
	    c2numpy_float64(&writer, cand->eta());
	    c2numpy_float64(&writer, cand->phi());
	    c2numpy_intc(&writer, cand->pdgId());
	    c2numpy_intc(&writer, charge);
	    c2numpy_intc(&writer, ak7_index);
  	  }
    
    ak7_index++;
    }  
    // Number of saved jets in the event
    njet_ak7 = ak7_index;

    // MET
    Handle< PFMETCollection > met_handle;
    event_obj.getByLabel("pfMet", met_handle);

    met = (*met_handle)[0].et();
    sumet = (*met_handle)[0].sumEt();

    // All PF Candidates
    
    //pfCandidates_ = edm::InputTag("particleFlow","","RECO");
    // Handle<std::vector<reco::PFCandidate> > pfCandidates;
    // event_obj.getByLabel(pfCandidates_, pfCandidates);

    // for(size_t i = 0; i < pfCandidates->size(); i++){
    //   reco::PFCandidate p = (*pfCandidates)[i];
    //   if (fabs(p.eta()) > 5.0) continue;
    //   pts->push_back(p.pt());
    //   etas->push_back(p.eta());
    //   phis->push_back(p.phi());
    //   ids->push_back(p.pdgId());
    // }

    // Finally, fill the tree
    if ( njet_ak7 >= (unsigned)mMinNPFJets ) {            
            mTree->Fill();
    }

}


void OpenDataTreeProducerOptimized::endRun(edm::Run const &iRun, edm::EventSetup const &iSetup) {

}

OpenDataTreeProducerOptimized::~OpenDataTreeProducerOptimized() {
}


//float OpenDataTreeProducerOptimized::getTau(unsigned num, const edm::Ptr<reco::Jet> & object) const
float OpenDataTreeProducerOptimized::getTau(unsigned num, const reco::PFJet * object) const
{
  std::vector<fastjet::PseudoJet> FJparticles;
  for (unsigned k = 0; k < object->numberOfDaughters(); ++k)
    {
      const reco::CandidatePtr & dp = object->daughterPtr(k);
      if ( dp.isNonnull() && dp.isAvailable() )
	FJparticles.push_back( fastjet::PseudoJet( dp->px(), dp->py(), dp->pz(), dp->energy() ) );
      else
	edm::LogWarning("MissingJetConstituent") << "Jet constituent required for N-subjettiness computation is missing!";
    }

  return routine_->getTau(num, FJparticles); 
}




DEFINE_FWK_MODULE(OpenDataTreeProducerOptimized);
