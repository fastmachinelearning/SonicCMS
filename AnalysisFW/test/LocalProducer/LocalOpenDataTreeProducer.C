#define LocalOpenDataTreeProducer_cxx
#include "LocalOpenDataTreeProducer.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

bool compareTrgNames(std::string left, std::string right) {

    return  std::stoi(left.substr(7, std::string::npos)) <
            std::stoi(right.substr(7, std::string::npos)); 
}

std::string transformName(std::string str) {
    return "jt" + str.substr(7, string::npos);
}

std::string removeTrgVersion(std::string &trg_name) {

    size_t last_i = trg_name.find_last_of("_");
    return trg_name.substr(0, last_i);
}


void LocalOpenDataTreeProducer::Loop()
{
//   In a ROOT session, you can do:
//      root> .L LocalOpenDataTreeProducer.C
//      root> LocalOpenDataTreeProducer t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch

    if (fChain == 0) return;

    TDirectory *currDir = gDirectory;
    TFile *fout = new TFile("./OpenDataTree.root", "RECREATE");
    TTree *tree = new TTree("OpenDataTree", "OpenDataTree");
    currDir->cd();

    const UInt_t kMaxNjet = 100;
    const UInt_t kMaxNtrg = 16;

    UInt_t njet;
    Float_t jet_pt[kMaxNjet];
    Float_t jet_eta[kMaxNjet];
    Float_t jet_phi[kMaxNjet];
    Float_t jet_E[kMaxNjet];
    Bool_t jet_tightID[kMaxNjet];
    Float_t jet_area[kMaxNjet];
    Float_t jet_jes[kMaxNjet];

    
    Bool_t isMC = true;

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
    //char triggernames[64][kMaxNtrg];
    UInt_t prescales[kMaxNtrg];

    // MET, SuMET, rho, eventfilter
    Float_t met;
    Float_t sumet;
    Float_t rho;
    Float_t pthat;
    Float_t mcweight;


    TBranch *b_njet = tree->Branch("njet", &njet, "njet/i");
    TBranch *b_jet_pt = tree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    TBranch *b_jet_eta = tree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    TBranch *b_jet_phi = tree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    TBranch *b_jet_E = tree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    TBranch *b_jet_tightID = tree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    TBranch *b_jet_area = tree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    TBranch *b_jet_jes = tree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
    

    if (isMC) {
        TBranch *b_ngen = tree->Branch("ngen", &ngen, "ngen/i");
        TBranch *b_gen_pt = tree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
        TBranch *b_gen_eta = tree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
        TBranch *b_gen_phi = tree->Branch("gen_phi", gen_eta, "gen_phi[ngen]/F");
        TBranch *b_gen_E = tree->Branch("gen_E", gen_E, "gen_E[ngen]/F");
    }

    TBranch *b_run = tree->Branch("run", &run, "run/i");
    TBranch *b_lumi = tree->Branch("lumi", &lumi, "lumi/i");
    TBranch *b_event = tree->Branch("event", &event, "event/l");

    TBranch *b_ntrg = tree->Branch("ntrg", &ntrg, "ntrg/i");
    TBranch *b_triggers = tree->Branch("triggers", triggers, "triggers[ntrg]/O");
    TBranch *b_triggernames = tree->Branch("triggernames", &triggernames);
    TBranch *b_prescales = tree->Branch("prescales", prescales, "prescales[ntrg]/i");


    TBranch *b_met = tree->Branch("met", &met, "met/F");
    TBranch *b_sumet = tree->Branch("sumet", &sumet, "sumet/F");
    TBranch *b_rho = tree->Branch("rho", &rho, "rho/F");
    TBranch *b_pthat = tree->Branch("pthat", &pthat, "pthat/F");
    TBranch *b_mcweight = tree->Branch("mcweight", &mcweight, "mcweight/F");


    fChain->SetBranchStatus("*",0);  // disable all branches
    
    fChain->SetBranchStatus("PFJets_",1); // njet
    fChain->SetBranchStatus("PFJets_.P4_.fCoordinates.fX",1);
    fChain->SetBranchStatus("PFJets_.P4_.fCoordinates.fY",1);
    fChain->SetBranchStatus("PFJets_.P4_.fCoordinates.fZ",1);
    fChain->SetBranchStatus("PFJets_.P4_.fCoordinates.fT",1);
    fChain->SetBranchStatus("PFJets_.tightID_",1); // jet_tightID
    fChain->SetBranchStatus("PFJets_.area_",1); // jet_area
    fChain->SetBranchStatus("PFJets_.cor_",1); // jet_jes
    
    
    if (isMC) { // MC
    fChain->SetBranchStatus("GenJets_",1); // ngen
    fChain->SetBranchStatus("GenJets_.fCoordinates.fX",1);
    fChain->SetBranchStatus("GenJets_.fCoordinates.fY",1);
    fChain->SetBranchStatus("GenJets_.fCoordinates.fZ",1);
    fChain->SetBranchStatus("GenJets_.fCoordinates.fT",1);
    }
    

    fChain->SetBranchStatus("TriggerDecision_",1);
    fChain->SetBranchStatus("L1Prescale_",1);
    fChain->SetBranchStatus("HLTPrescale_",1);


    fChain->SetBranchStatus("EvtHdr_.mRun",1); // run
    fChain->SetBranchStatus("EvtHdr_.mLumi",1); // lumi
    fChain->SetBranchStatus("EvtHdr_.mEvent",1); // event
    fChain->SetBranchStatus("PFMet_.et_",1); // met
    fChain->SetBranchStatus("PFMet_.sumEt_",1); // sumet
    fChain->SetBranchStatus("EvtHdr_.mPFRho",1); // rho
    fChain->SetBranchStatus("EvtHdr_.mPthat",1); // pthat
    fChain->SetBranchStatus("EvtHdr_.mWeight",1); // mcweight


    TLorentzVector p4, p4gen;
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t nbytes = 0, nb = 0;
 
    // Process triggers only for actual data
    if (!isMC) {    
        // Trigger names, common to all events
        auto trg_list = TriggerNames->GetXaxis()->GetLabels();

        // Helper variables, not included in the output
        set<std::string> s;
        for (int i = 0; i != trg_list->GetSize(); ++i ) {

            std::string trg_name(((TObjString*)trg_list->At(i))->GetName());

            // Mapping input trigger indexes to output trigger list
            // "HLT_Jet150_v3" into "HLT_Jet150" 
            s.insert(removeTrgVersion(trg_name));
        }

        // Convert set into vector
        std::vector<std::string> trg_vec;
        trg_vec.assign( s.begin(), s.end() );

        // And sort the vector wrt. the trigger momentum 
        sort(trg_vec.begin(), trg_vec.end(), compareTrgNames);

        // Number of triggers
        ntrg = trg_vec.size();

        // Shorter trigger names for output
        for (auto i : trg_vec ) {
            triggernames.push_back(transformName(i));
        }
    }
    
    // Iterating over the events
    for (Long64_t jentry = 0; jentry != nentries; ++jentry) {
        
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;          
        nb = fChain->GetEntry(jentry);   
        nbytes += nb;

        njet = PFJets__;
        for (int i = 0; i != njet; ++i) {

            p4.SetPxPyPzE(  PFJets__P4__fCoordinates_fX[i], PFJets__P4__fCoordinates_fY[i],
                            PFJets__P4__fCoordinates_fZ[i], PFJets__P4__fCoordinates_fT[i]);
        
            jet_pt[i] = p4.Pt()/PFJets__cor_[i];    
            jet_eta[i] = p4.Eta();
            jet_phi[i] = p4.Phi();
            jet_E[i] = p4.E()/PFJets__cor_[i];

            jet_tightID[i] = PFJets__tightID_[i];
            jet_area[i] = PFJets__area_[i];
            jet_jes[i] = PFJets__cor_[i]; // NOTE!! Energy scale, not correction (!)
        }


    
        // MC jets
        if (isMC) {
            ngen = GenJets__;
            pthat = EvtHdr__mPthat;
            mcweight = EvtHdr__mWeight;

            for (int i = 0; i != ngen; ++i) {
                p4gen.SetPxPyPzE(   GenJets__fCoordinates_fX[i], GenJets__fCoordinates_fY[i],
                                    GenJets__fCoordinates_fZ[i], GenJets__fCoordinates_fT[i]);
                
                gen_pt[i] = p4gen.Pt(); 
                gen_eta[i] = p4gen.Eta();
                gen_phi[i] = p4gen.Phi();
                gen_E[i] = p4gen.E();
            }
        }
        

        // Triggers and prescales

        /*
        auto trg_list = TriggerNames->GetXaxis()->GetLabels();
        for (int itrg = 0; itrg != trg_list->GetSize(); ++itrg ) {
            
            int pass = TriggerDecision_[itrg]; // -1, 0, 1
            if (pass == -1) {
                continue;
            }
            else { 
                std::string trg_name(((TObjString*)trg_list->At(itrg))->GetName());

                int pos = find(trg_vec.begin(), trg_vec.end(), removeTrgVersion(trg_name)) - trg_vec.begin();
                triggers[pos] = TriggerDecision_[itrg];
                prescales[pos] = HLTPrescale_[itrg]*L1Prescale_[itrg];
            }
        }
        */
        
        // Event identification
        run = EvtHdr__mRun;
        event = EvtHdr__mEvent;
        lumi = EvtHdr__mLumi;

        // MET, SuMET, rho, eventfilter
        met = PFMet__et_;
        sumet = PFMet__sumEt_;
        rho = EvtHdr__mPFRho;
        //eventfilter = ;  ???

        tree->Fill();
   }

   fout->cd();
   tree->Write();
   fout->Close();
}
