

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

OpenDataTreeProducerOptimized::OpenDataTreeProducerOptimized(edm::ParameterSet const &cfg) :
  JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
  JetTok_(consumes<edm::View<pat::Jet>>(JetTag_))
{
  mMinPFPt           = cfg.getParameter<double>                    ("minPFPt");
  mMinJJMass         = cfg.getParameter<double>                    ("minJJMass");
  mMaxY              = cfg.getParameter<double>                    ("maxY");
  mMaxEta            = cfg.getParameter<double>                    ("maxEta");
  mMinNPFJets        = cfg.getParameter<int>                       ("minNPFJets");
  mMaxRows           = cfg.getUntrackedParameter<int>              ("maxRows",1000000);
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


  
  // measureDefinition_ = 0; //CMS default is normalized measure
  // beta_ = 1.0; //CMS default is 1
  // R0_ = 0.7; // CMS default is jet cone size
  // Rcutoff_ = -999.0; // not used by default
  // // variables for axes definition
  // axesDefinition_ = 6; // CMS default is 1-pass KT axes
  // nPass_ = -999; // not used by default
  // akAxesR0_ = -999.0; // not used by default
  // // for softdrop
  // zCut_ = 0.1;
    
  // // Get the measure definition
  // fastjet::contrib::NormalizedMeasure          normalizedMeasure        (beta_,R0_);
  // fastjet::contrib::UnnormalizedMeasure        unnormalizedMeasure      (beta_);
  // // fastjet::contrib::GeometricMeasure           geometricMeasure         (beta_);
  // fastjet::contrib::NormalizedCutoffMeasure    normalizedCutoffMeasure  (beta_,R0_,Rcutoff_);
  // fastjet::contrib::UnnormalizedCutoffMeasure  unnormalizedCutoffMeasure(beta_,Rcutoff_);
  // // fastjet::contrib::GeometricCutoffMeasure     geometricCutoffMeasure   (beta_,Rcutoff_);

  // fastjet::contrib::MeasureDefinition const * measureDef = 0;
  // switch ( measureDefinition_ ) {
  // case UnnormalizedMeasure : measureDef = &unnormalizedMeasure; break;
  // // case GeometricMeasure    : measureDef = &geometricMeasure; break;
  // case NormalizedCutoffMeasure : measureDef = &normalizedCutoffMeasure; break;
  // case UnnormalizedCutoffMeasure : measureDef = &unnormalizedCutoffMeasure; break;
  // // case GeometricCutoffMeasure : measureDef = &geometricCutoffMeasure; break;
  // case NormalizedMeasure : default : measureDef = &normalizedMeasure; break;
  // } 

  // // Get the axes definition
  // fastjet::contrib::KT_Axes             kt_axes; 
  // fastjet::contrib::CA_Axes             ca_axes; 
  // fastjet::contrib::AntiKT_Axes         antikt_axes   (akAxesR0_);
  // fastjet::contrib::WTA_KT_Axes         wta_kt_axes; 
  // fastjet::contrib::WTA_CA_Axes         wta_ca_axes; 
  // fastjet::contrib::OnePass_KT_Axes     onepass_kt_axes;
  // fastjet::contrib::OnePass_CA_Axes     onepass_ca_axes;
  // fastjet::contrib::OnePass_AntiKT_Axes onepass_antikt_axes   (akAxesR0_);
  // fastjet::contrib::OnePass_WTA_KT_Axes onepass_wta_kt_axes;
  // fastjet::contrib::OnePass_WTA_CA_Axes onepass_wta_ca_axes;
  // fastjet::contrib::MultiPass_Axes      multipass_axes (nPass_);

  // fastjet::contrib::AxesDefinition const * axesDef = 0;
  // switch ( axesDefinition_ ) {
  // case  KT_Axes : default : axesDef = &kt_axes; break;
  // case  CA_Axes : axesDef = &ca_axes; break; 
  // case  AntiKT_Axes : axesDef = &antikt_axes; break;
  // case  WTA_KT_Axes : axesDef = &wta_kt_axes; break; 
  // case  WTA_CA_Axes : axesDef = &wta_ca_axes; break; 
  // case  OnePass_KT_Axes : axesDef = &onepass_kt_axes; break;
  // case  OnePass_CA_Axes : axesDef = &onepass_ca_axes; break; 
  // case  OnePass_AntiKT_Axes : axesDef = &onepass_antikt_axes; break;
  // case  OnePass_WTA_KT_Axes : axesDef = &onepass_wta_kt_axes; break; 
  // case  OnePass_WTA_CA_Axes : axesDef = &onepass_wta_ca_axes; break; 
  // case  MultiPass_Axes : axesDef = &multipass_axes; break;
  // };

  // routine_ = std::auto_ptr<fastjet::contrib::Njettiness> ( new fastjet::contrib::Njettiness( *axesDef, *measureDef ) );
  
}

void OpenDataTreeProducerOptimized::beginJob() {
  
    etas   = new std::vector<float>; etas->clear();
    phis   = new std::vector<float>; phis->clear();
    pts    = new std::vector<float>; pts->clear();
    ids    = new std::vector<int>;   ids->clear();    
    charges= new std::vector<int>;   charges->clear();    
    ak7indices    = new std::vector<int>;   ak7indices->clear();
    
   //  mTree = fs->make< TTree >("OpenDataTree", "OpenDataTree");

   //  // Variables of the flat tuple
   //  // mTree->Branch("njet", &njet, "njet/i");
   //  // mTree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
   //  // mTree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
   //  // mTree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
   //  // mTree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
   //  // mTree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
   //  // mTree->Branch("jet_area", jet_area, "jet_area[njet]/F");
   //  // mTree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
   //  // mTree->Branch("jet_igen", jet_igen, "jet_igen[njet]/I");

   //  // AK7 variables
   //  mTree->Branch("njet_ak7", &njet_ak7, "njet_ak7/i");
   //  mTree->Branch("jet_pt_ak7", jet_pt_ak7, "jet_pt_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_eta_ak7", jet_eta_ak7, "jet_eta_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_phi_ak7", jet_phi_ak7, "jet_phi_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_E_ak7", jet_E_ak7, "jet_E_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_msd_ak7", jet_msd_ak7, "jet_msd_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_area_ak7", jet_area_ak7, "jet_area_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_jes_ak7", jet_jes_ak7, "jet_jes_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_tau21_ak7", jet_tau21_ak7, "jet_tau21_ak7[njet_ak7]/F");
   //  mTree->Branch("jet_igen_ak7", jet_igen_ak7, "jet_igen_ak7[njet_ak7]/I");
   //  mTree->Branch("jet_igenjet_ak7", jet_igenjet_ak7, "jet_igenjet_ak7[njet_ak7]/I");
   //  mTree->Branch("jet_isW_ak7", jet_isW_ak7, "jet_isW_ak7[njet_ak7]/I");
   //  mTree->Branch("jet_ncand_ak7", jet_ncand_ak7, "jet_ncand_ak7[njet_ak7]/I");
   //  //mTree->Branch("ak7_to_ak5", ak7_to_ak5, "ak7_to_ak5[njet_ak7]/I");

   //  mTree->Branch("ngen", &ngen, "ngen/i");
   //  mTree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
   //  mTree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
   //  mTree->Branch("gen_phi", gen_phi, "gen_phi[ngen]/F");
   //  mTree->Branch("gen_E", gen_E, "gen_E[ngen]/F");
    
   //  mTree->Branch("ngenparticles", &ngenparticles, "ngenparticles/i");
   //  mTree->Branch("genparticle_pt", genparticle_pt, "genparticle_pt[ngenparticles]/F");
   //  mTree->Branch("genparticle_eta", genparticle_eta, "genparticle_eta[ngenparticles]/F");
   //  mTree->Branch("genparticle_phi", genparticle_phi, "genparticle_phi[ngenparticles]/F");
   //  mTree->Branch("genparticle_E", genparticle_E, "genparticle_E[ngenparticles]/F");
   //  mTree->Branch("genparticle_id", genparticle_id, "genparticle_id[ngenparticles]/I");
   //  mTree->Branch("genparticle_dauId1", genparticle_dauId1, "genparticle_dauId1[ngenparticles]/I");
   //  mTree->Branch("genparticle_dauId2", genparticle_dauId2, "genparticle_dauId2[ngenparticles]/I");
   //  mTree->Branch("genparticle_dauDR", genparticle_dauDR, "genparticle_dauDR[ngenparticles]/F");
   //  mTree->Branch("genparticle_status", genparticle_status, "genparticle_status[ngenparticles]/I");

   //  mTree->Branch("run", &run, "run/i");
   //  mTree->Branch("lumi", &lumi, "lumi/i");
   //  mTree->Branch("event", &event, "event/l");
   //  mTree->Branch("ntrg", &ntrg, "ntrg/i");
   //  mTree->Branch("triggers", triggers, "triggers[ntrg]/O");
   //  mTree->Branch("triggernames", &triggernames);
   //  //mTree->Branch("prescales", prescales, "prescales[ntrg]/i");
   //  mTree->Branch("met", &met, "met/F");
   //  mTree->Branch("sumet", &sumet, "sumet/F");
   //  mTree->Branch("rho", &rho, "rho/F");
   //  mTree->Branch("pthat", &pthat, "pthat/F");
   //  mTree->Branch("mcweight", &mcweight, "mcweight/F");

   //  // mTree->Branch("chf", chf, "chf[njet]/F");   
   //  // mTree->Branch("nhf", nhf, "nhf[njet]/F");   
   //  // mTree->Branch("phf", phf, "phf[njet]/F");   
   //  // mTree->Branch("elf", elf, "elf[njet]/F");   
   //  // mTree->Branch("muf", muf, "muf[njet]/F");   
   //  // mTree->Branch("hf_hf", hf_hf, "hf_hf[njet]/F");   
   //  // mTree->Branch("hf_phf", hf_phf, "hf_phf[njet]/F");   
   //  // mTree->Branch("hf_hm", hf_hm, "hf_hm[njet]/i");    
   //  // mTree->Branch("hf_phm", hf_phm, "hf_phm[njet]/i");
   //  // mTree->Branch("chm", chm, "chm[njet]/i");   
   //  // mTree->Branch("nhm", nhm, "nhm[njet]/i");   
   //  // mTree->Branch("phm", phm, "phm[njet]/i");   
   //  // mTree->Branch("elm", elm, "elm[njet]/i");   
   //  // mTree->Branch("mum", mum, "mum[njet]/i");
   //  // mTree->Branch("beta", beta, "beta[njet]/F");   
   //  // mTree->Branch("bstar", bstar, "bstar[njet]/F");

   //  //PF Candidates
   //  mTree->Branch("ak7pfcand_pt", "std::vector<float>", &pts);
   //  mTree->Branch("ak7pfcand_eta", "std::vector<float>", &etas);
   //  mTree->Branch("ak7pfcand_phi", "std::vector<float>", &phis);
   //  mTree->Branch("ak7pfcand_id", "std::vector<int>", &ids);
   //  mTree->Branch("ak7pfcand_charge", "std::vector<int>", &charges);
   //  mTree->Branch("ak7pfcand_ijet", "std::vector<int>", &ak7indices);

    
   //  //c2numpy
   // c2numpy_init(&writer, "params", mMaxRows);
   // c2numpy_addcolumn(&writer, "run", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "lumi", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "event", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "met", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "sumet", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "rho", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "pthat", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "mcweight", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "njet_ak7", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "jet_pt_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_ptraw_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_eta_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_phi_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_E_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_Eraw_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_msd_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_area_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_jes_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_tau21_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "jet_isW_ak7", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "jet_ncand_ak7", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "genjet_pt_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "genjet_eta_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "genjet_phi_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "genjet_E_ak7", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "ak7pfcand_pt", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "ak7pfcand_eta", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "ak7pfcand_phi", C2NUMPY_FLOAT64);
   // c2numpy_addcolumn(&writer, "ak7pfcand_id", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "ak7pfcand_charge", C2NUMPY_INTC);
   // c2numpy_addcolumn(&writer, "ak7pfcand_ijet", C2NUMPY_INTC);

    
}

void OpenDataTreeProducerOptimized::endJob() {
  // c2numpy
  c2numpy_close(&writer);
}


void OpenDataTreeProducerOptimized::beginRun(edm::Run const &iRun,
                                     edm::EventSetup const &iSetup) { 

  // load the graph 
  tensorflow::setLogging();
  graphDef_ = tensorflow::loadGraphDef("resnet50.pb");

}


void OpenDataTreeProducerOptimized::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {
  
    etas->clear();
    phis->clear();
    pts->clear();
    ids->clear();    
    charges->clear();    
    ak7indices->clear();

    edm::Handle<edm::View<pat::Jet>> h_jets;
    event_obj.getByToken(JetTok_, h_jets);

    // create a jet image for the leading jet in the event
    // 224 x 224 image which is centered at the jet axis and +/- 1 unit in eta and phi
    float image2D[224][224];
    float pixel_width = 2./224.;
    for (int ii = 0; ii < 224; ii++){
      for (int ij = 0; ij < 224; ij++){ image2D[ii][ij] = 0.; }
    }
    
    int jet_ctr = 0;
    for(const auto& i_jet : *(h_jets.product())){
      
      //jet calcs
      float jet_pt  =  i_jet.pt();
      float jet_phi =  i_jet.phi();
      float jet_eta =  i_jet.eta();

      for(unsigned k = 0; k < i_jet.numberOfDaughters(); ++k){

        const reco::Candidate* i_part = i_jet.daughter(k);
        // daughter info
        float i_pt = i_part->pt();
        float i_phi = i_part->phi();
        float i_eta = i_part->eta();
        //std::cout << "daughter pt = " << i_pt << std::endl;
        
        float dphi = i_phi - jet_phi;
        if (dphi > M_PI) dphi -= 2*M_PI;
        if (dphi < -1.*M_PI) dphi += 2*M_PI;
        float deta = i_eta - jet_eta;

        if ( deta > 1. || deta < -1. || dphi > 1. || dphi < 1.) continue; // outside of the image, shouldn't happen for AK8 jet!
        int eta_pixel_index =  (int) ((deta + 1.)/pixel_width);
        int phi_pixel_index =  (int) ((dphi + 1.)/pixel_width);
        image2D[eta_pixel_index][phi_pixel_index] += i_pt/jet_pt;

      }
  
      //////////////////////////////
      jet_ctr++;
      if (jet_ctr > 0) break; // just do one jet for now
      //////////////////////////////

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
