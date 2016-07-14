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

#include "SMPJ/AnalysisFW/plugins/OpenDataTreeProducer.h"
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

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"

OpenDataTreeProducer::OpenDataTreeProducer(edm::ParameterSet const &cfg) {
  mPFPayloadName     = cfg.getParameter<std::string>               ("PFPayloadName");
  mMinPFPt           = cfg.getParameter<double>                    ("minPFPt");
  mMinJJMass         = cfg.getParameter<double>                    ("minJJMass");
  mMaxY              = cfg.getParameter<double>                    ("maxY");
  mMinNPFJets        = cfg.getParameter<int>                       ("minNPFJets");
  mPFJetsName        = cfg.getParameter<edm::InputTag>             ("pfjets");
  mSrcPFRho          = cfg.getParameter<edm::InputTag>             ("srcPFRho");
  mPFMET             = cfg.getParameter<edm::InputTag>             ("pfmet");
  mGenJetsName       = cfg.getUntrackedParameter<edm::InputTag>    ("genjets",edm::InputTag(""));
  mPrintTriggerMenu  = cfg.getUntrackedParameter<bool>             ("printTriggerMenu",false);
  mIsMCarlo          = cfg.getUntrackedParameter<bool>             ("isMCarlo",false);
  mUseGenInfo        = cfg.getUntrackedParameter<bool>             ("useGenInfo",false);
  mMinGenPt          = cfg.getUntrackedParameter<double>           ("minGenPt",30);
  processName_       = cfg.getParameter<std::string>               ("processName");
  triggerNames_      = cfg.getParameter<std::vector<std::string> > ("triggerNames");
  triggerResultsTag_ = cfg.getParameter<edm::InputTag>             ("triggerResults");
}
//////////////////////////////////////////////////////////////////////////////////////////
void OpenDataTreeProducer::beginJob() {
    mTree = fs->make< TTree >("OpenDataTree", "OpenDataTree");

    // Variables to be written for each event
    mTree->Branch("njet", &njet, "njet/i");
    mTree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    mTree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    mTree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    mTree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    mTree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    mTree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    mTree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
    mTree->Branch("jet_igen", jet_igen, "jet_igen[njet]/I");

    mTree->Branch("ngen", &ngen, "ngen/i");
    mTree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
    mTree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
    mTree->Branch("gen_phi", gen_phi, "gen_phi[ngen]/F");
    mTree->Branch("gen_E", gen_E, "gen_E[ngen]/F");

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

}

//////////////////////////////////////////////////////////////////////////////////////////

void OpenDataTreeProducer::endJob() {
}
//////////////////////////////////////////////////////////////////////////////////////////

void OpenDataTreeProducer::beginRun(edm::Run const &iRun,
                                     edm::EventSetup const &iSetup) {

    //--------------------------------------------------------------
    //-------------- Mapping trigger indexes -----------------------
    
    bool changed(true);
    if (hltConfig_.init(iRun, iSetup, processName_, changed) && changed){

        // List of trigger names and indices are not emptied between events, 
        // must be done here
        triggerIndex_.clear();
        triggernames.clear();

        // Iterate over all active triggers of the AOD file
        auto name_list = hltConfig_.triggerNames();
        for (std::string name_to_search: triggerNames_) {

            // Find the version of jet trigger that is active in this run 
            for (std::string name_candidate: name_list) {

                // Match the prefix to the full name
                if ( name_candidate.find(name_to_search) != std::string::npos ) {

                    // Save index corresponding to the trigger
                    triggerIndex_.push_back(hltConfig_.triggerIndex(name_candidate));

                    // Save the trigger name
                    triggernames.push_back("jt" + name_to_search.substr(7, string::npos));
                    //triggernames.push_back(name_candidate);
                    break;            
                }
            }
        }
    }

    // Retrieve cross section of the simulated process
    if (mIsMCarlo) {

        edm::Handle<GenRunInfoProduct> genRunInfo;
        iRun.getByLabel("generator", genRunInfo );

        // Save only the cross section, 
        // since the number of events is not available in this context (afaik) 
        mcweight = genRunInfo->crossSection();
        std::cout << "Cross section: " <<  mcweight << std::endl;
    }
    

}


//////////////////////////////////////////////////////////////////////////////////////////

void OpenDataTreeProducer::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {

    //--------------------------------------------------------------
    //-------------- Basic Event Info ------------------------------
    run = event_obj.id().run();
    lumi = event_obj.luminosityBlock();
    event = event_obj.id().event();

    //--------------------------------------------------------------
    //-------------- Trigger Info ----------------------------------

    // Retrieve information from the event
    edm::Handle<edm::TriggerResults>   triggerResultsHandle_;
    event_obj.getByLabel(triggerResultsTag_, triggerResultsHandle_);

    // Sanity checks
    assert(triggerResultsHandle_.isValid() && "Error in getting TriggerResults from Event!");
    assert(triggerResultsHandle_->size() == hltConfig_.size() && "Size mismatch between triggerResultsHandle_ and hltConfig_");
    
    // Number of triggers to be saved
    ntrg = triggerIndex_.size();

    // Iterate only over the selected jet triggers
    for (unsigned itrig = 0; itrig < ntrg && !mIsMCarlo; itrig++) {

        // Trigger bit
        Bool_t isAccepted = triggerResultsHandle_->accept(triggerIndex_[itrig]);
        triggers[itrig] = isAccepted;

        // Trigger prescales, must be retrieved using the trigger name
        std::string trgName = hltConfig_.triggerName(triggerIndex_[itrig]);
        const std::pair< int, int > prescalePair(hltConfig_.prescaleValues(event_obj, iSetup, trgName));
        
        // Total prescale: PreL1*PreHLT 
        prescales[itrig] = prescalePair.first*prescalePair.second; // PreL1*PreHLT    
    }    


    //------------------------------------------------------------------
    //-------------- Rho -----------------------------------------------
    Handle< double > rho_handle;
    event_obj.getByLabel(mSrcPFRho, rho_handle);
    rho = *rho_handle;


    //-------------- Generator Info -------------------------------------

    // Retrieve pthat and mcweight for MC datasets
    if (mIsMCarlo && mUseGenInfo) {
        // Handle
        Handle< GenEventInfoProduct > hEventInfo;
        event_obj.getByLabel("generator", hEventInfo);

        // Monte Carlo weight (NOT AVAILABLE FOR 2011 MC!!)
        //mcweight = hEventInfo->weight();
        
        // Pthat value
        if (hEventInfo->hasBinningValues()) {
            pthat = hEventInfo->binningValues()[0];
        }
    }


    //-----------------------------------------------------------------
    // ----------------- GenJets --------------------------------------

    // Save generated jets
    if (mIsMCarlo) {

        // Retrieve collection of simulated jets
        Handle< GenJetCollection > genjets;
        event_obj.getByLabel(mGenJetsName, genjets);
    
        // Index of the simulated jet
        int gen_index = 0; 

        for (GenJetCollection::const_iterator i_gen = genjets->begin(); i_gen != genjets->end(); i_gen++)  {

            // Jet selection
            if (i_gen->pt() > mMinGenPt && fabs(i_gen->y()) < mMaxY) {
                gen_pt[gen_index] = i_gen->pt();
                gen_eta[gen_index] = i_gen->eta();
                gen_phi[gen_index] = i_gen->phi();
                gen_E[gen_index] = i_gen->energy();
                gen_index++;
            }
        }

        // Total number of generated jet in this event
        ngen = gen_index;
    }


    //------------------------------------------------------
    //----------- PFJets (non-CHS) -------------------------

    // Retrieve jet information
    edm::Handle< std::vector< pat::Jet > > jet_handle;
    event_obj.getByLabel(mPFJetsName, jet_handle);

    // Copy vector of jets to sort them wrt. pT
    std::vector< pat::Jet > patjets(jet_handle->begin(), jet_handle->end());
    
    // Sort jets, does it need to be done ¿¿??
    //std::sort(patjets.begin(), patjets.end(), cmp_patjets); 

    // Index of the selected jet 
    int jet_index = 0;

    // Iterate over the jets of the event
    for (auto i_pfjet = patjets.begin(); i_pfjet != patjets.end(); ++i_pfjet) {

        // Skip the current iteration if jet is not selected
        if (!i_pfjet->isPFJet() || 
            fabs(i_pfjet->y()) > mMaxY || 
            (i_pfjet->pt()*i_pfjet->jecFactor(0)) < mMinPFPt) {
            continue;
        }

        // Jet properties
        int     npr     = i_pfjet->chargedMultiplicity() + 
                            i_pfjet->neutralMultiplicity();
        double  phf     = i_pfjet->photonEnergyFraction();
        double  nhf     = i_pfjet->neutralHadronEnergyFraction() +
                            i_pfjet->HFHadronEnergyFraction();
        double  elf     = i_pfjet->electronEnergyFraction();
        double  chf     = i_pfjet->chargedHadronEnergyFraction();
        int     chm     = i_pfjet->chargedHadronMultiplicity();

        bool isHighEta = fabs(i_pfjet->eta()) > 2.4;
        bool isLowEta = fabs(i_pfjet->eta()) <= 2.4 && 
                        nhf < 0.9 &&
                        phf < 0.9 && 
                        elf < 0.99 && 
                        chf > 0 && 
                        chm > 0;
        bool tightID =  npr > 1 && 
                        phf < 0.99 && 
                        nhf < 0.99 &&
                        (isLowEta || isHighEta);


        // Variables to be saved
        jet_tightID[jet_index] = tightID;
        jet_area[jet_index] = i_pfjet->jetArea();
        jet_jes[jet_index] = 1./i_pfjet->jecFactor(0); // JEC energy scale

        double correction = i_pfjet->jecFactor(0);
        jet_pt[jet_index] = i_pfjet->pt()*correction;  // Store corrected values
        jet_eta[jet_index] = i_pfjet->eta();
        jet_phi[jet_index] = i_pfjet->phi();
        jet_E[jet_index] = i_pfjet->energy()*correction; // Store corrected values
        

        // Matching a GenJet to this PFjet
        if (mIsMCarlo && ngen > 0) {

            // Index of the generated jet matching this PFjet
            jet_igen[jet_index] = -1; // -1, if no matching jet

            // Search the generated jet with minimum distance to this PFjet   
            float r2min(999);
            for (unsigned int gen_index = 0; gen_index != ngen; gen_index++) {
                double deltaR2 = reco::deltaR2( jet_eta[jet_index], 
                                                jet_phi[jet_index],
                                                gen_eta[gen_index], 
                                                gen_phi[gen_index]);
                if (deltaR2 < r2min) {
                    r2min = deltaR2;
                    jet_igen[jet_index] = gen_index;
                }
            }
        }
        
    jet_index++;
    }  
    // Number of selected jets in the event
    njet = jet_index;    
    

    //------------------------------------------------------------------
    //---------------- MET ---------------------------------------------
    Handle< PFMETCollection > met_handle;
    event_obj.getByLabel("pfMet", met_handle);
    met = (*met_handle)[0].et();
    sumet = (*met_handle)[0].sumEt();

    //-----------------------------------------------------------------
    //-------------- Fill the ree -------------------------------------
    if (njet >= (unsigned)mMinNPFJets) {            
            mTree->Fill();
    }
}


void OpenDataTreeProducer::endRun(edm::Run const &iRun, edm::EventSetup const &iSetup) {

}

//////////////////////////////////////////////////////////////////////////////////////////
OpenDataTreeProducer::~OpenDataTreeProducer() {
}




DEFINE_FWK_MODULE(OpenDataTreeProducer);
