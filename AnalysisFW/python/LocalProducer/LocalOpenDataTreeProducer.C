#define LocalOpenDataTreeProducer_cxx
#include "LocalOpenDataTreeProducer.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>


// These functions depend on the trigger names !!
bool compareTrgNames(std::string left, std::string right) {

    return left < right;
    return  std::stoi(left.substr(7, std::string::npos)) <
            std::stoi(right.substr(7, std::string::npos)); 
}

std::string transformName(std::string str) {
    return str;
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

    if (fChain_ak4 == 0) return;

    TDirectory *currDir = gDirectory;
    TFile *fout = new TFile("./OpenDataTree.root", "RECREATE");
    TTree *tree = new TTree("OpenDataTree", "OpenDataTree");
    currDir->cd();

    const UInt_t kMaxNjet = 64;
    const UInt_t kMaxNtrg = 64;

    // PF AK5 jets
    UInt_t ak4_njet;
    Float_t ak4_pt[kMaxNjet];
    Float_t ak4_eta[kMaxNjet];
    Float_t ak4_phi[kMaxNjet];
    Float_t ak4_E[kMaxNjet];
    Bool_t ak4_tightID[kMaxNjet];
    Float_t ak4_area[kMaxNjet];
    Float_t ak4_jes[kMaxNjet];
    Int_t ak4_igen[kMaxNjet];

    // PF AK7 jets
    UInt_t ak7_njet;
    Float_t ak7_pt[kMaxNjet];
    Float_t ak7_eta[kMaxNjet];
    Float_t ak7_phi[kMaxNjet];
    Float_t ak7_E[kMaxNjet];
    Float_t ak7_area[kMaxNjet];
    Float_t ak7_jes[kMaxNjet];
    Int_t ak7_to_ak4[kMaxNjet];

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
    Float_t hof[kMaxPFJets_];  
    Float_t qgl[kMaxPFJets_];

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
    //char triggernames[64][kMaxNtrg];
    UInt_t prescales[kMaxNtrg];

    // MET, SuMET, rho, eventfilter
    Float_t met;
    Float_t sumet;
    Float_t rho;

    // MC variables
    Float_t pthat;
    Float_t mcweight;

    Bool_t isMC = false;

    TBranch *b_ak4_njet = tree->Branch("ak4_njet", &ak4_njet, "ak4_njet/i");
    TBranch *b_ak4_pt = tree->Branch("ak4_pt", ak4_pt, "ak4_pt[ak4_njet]/F");
    TBranch *b_ak4_eta = tree->Branch("ak4_eta", ak4_eta, "ak4_eta[ak4_njet]/F");
    TBranch *b_ak4_phi = tree->Branch("ak4_phi", ak4_phi, "ak4_phi[ak4_njet]/F");
    TBranch *b_ak4_E = tree->Branch("ak4_E", ak4_E, "ak4_E[ak4_njet]/F");   
    TBranch *b_ak4_tightID = tree->Branch("ak4_tightID", ak4_tightID, "ak4_tightID[ak4_njet]/O");
    TBranch *b_ak4_area = tree->Branch("ak4_area", ak4_area, "ak4_area[ak4_njet]/F");
    TBranch *b_ak4_jes = tree->Branch("ak4_jes", ak4_jes, "ak4_jes[ak4_njet]/F");
        
    TBranch *b_ak7_njet = tree->Branch("ak7_njet", &ak7_njet, "ak7_njet/i");
    TBranch *b_ak7_pt = tree->Branch("ak7_pt", ak7_pt, "ak7_pt[ak7_njet]/F");
    TBranch *b_ak7_eta = tree->Branch("ak7_eta", ak7_eta, "ak7_eta[ak7_njet]/F");
    TBranch *b_ak7_phi = tree->Branch("ak7_phi", ak7_phi, "ak7_phi[ak7_njet]/F");
    TBranch *b_ak7_E = tree->Branch("ak7_E", ak7_E, "ak7_E[ak7_njet]/F");   
    TBranch *b_ak7_area = tree->Branch("ak7_area", ak7_area, "ak7_area[ak7_njet]/F");
    TBranch *b_ak7_jes = tree->Branch("ak7_jes", ak7_jes, "ak7_jes[ak7_njet]/F");
    TBranch *b_ak7_to_ak4 = tree->Branch("ak7_to_ak4", ak7_to_ak4, "ak7_to_ak4[ak7_njet]/I");

    TBranch *b_chf = tree->Branch("chf", chf, "chf[ak4_njet]/F");   
   	TBranch *b_nhf = tree->Branch("nhf", nhf, "nhf[ak4_njet]/F");   
   	TBranch *b_phf = tree->Branch("phf", phf, "phf[ak4_njet]/F");   
   	TBranch *b_elf = tree->Branch("elf", elf, "elf[ak4_njet]/F");   
   	TBranch *b_muf = tree->Branch("muf", muf, "muf[ak4_njet]/F");   

   	TBranch *b_hf_hf = tree->Branch("hf_hf", hf_hf, "hf_hf[ak4_njet]/F");   
   	TBranch *b_hf_phf = tree->Branch("hf_phf", hf_phf, "hf_phf[ak4_njet]/F");   
   	TBranch *b_hf_hm = tree->Branch("hf_hm", hf_hm, "hf_hm[ak4_njet]/i");    
   	TBranch *b_hf_phm = tree->Branch("hf_phm", hf_phm, "hf_phm[ak4_njet]/i");
   	
    TBranch *b_chm = tree->Branch("chm", chm, "chm[ak4_njet]/i");   
   	TBranch *b_nhm = tree->Branch("nhm", nhm, "nhm[ak4_njet]/i");   
   	TBranch *b_phm = tree->Branch("phm", phm, "phm[ak4_njet]/i");   
   	TBranch *b_elm = tree->Branch("elm", elm, "elm[ak4_njet]/i");   
   	TBranch *b_mum = tree->Branch("mum", mum, "mum[ak4_njet]/i");
   
    TBranch *b_hof 		= tree->Branch("hof", hof, "hof[ak4_njet]/F");   
    TBranch *b_beta 	= tree->Branch("beta", beta, "beta[ak4_njet]/F");   
    TBranch *b_bstar 	= tree->Branch("bstar", bstar, "bstar[ak4_njet]/F");
    TBranch *b_qgl 		= tree->Branch("qgl", qgl, "qgl[ak4_njet]/F");
   	
    if (isMC) {
        TBranch *b_ngen 	= tree->Branch("ngen", &ngen, "ngen/i");
        TBranch *b_gen_pt 	= tree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
        TBranch *b_gen_eta 	= tree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
        TBranch *b_gen_phi 	= tree->Branch("gen_phi", gen_eta, "gen_phi[ngen]/F");
        TBranch *b_gen_E 	= tree->Branch("gen_E", gen_E, "gen_E[ngen]/F");
    }

    TBranch *b_run 		= tree->Branch("run", &run, "run/i");
    TBranch *b_lumi 	= tree->Branch("lumi", &lumi, "lumi/i");
    TBranch *b_event 	= tree->Branch("event", &event, "event/l");

    TBranch *b_ntrg 		= tree->Branch("ntrg", &ntrg, "ntrg/i");
    TBranch *b_triggers 	= tree->Branch("triggers", triggers, "triggers[ntrg]/O");
    TBranch *b_triggernames = tree->Branch("triggernames", &triggernames);
    TBranch *b_prescales 	= tree->Branch("prescales", prescales, "prescales[ntrg]/i");

    TBranch *b_met 		= tree->Branch("met", &met, "met/F");
    TBranch *b_sumet 	= tree->Branch("sumet", &sumet, "sumet/F");
    TBranch *b_rho 		= tree->Branch("rho", &rho, "rho/F");
    TBranch *b_pthat 	= tree->Branch("pthat", &pthat, "pthat/F");
    TBranch *b_mcweight = tree->Branch("mcweight", &mcweight, "mcweight/F");
    
    assert(fChain_ak4 && "AK4 tree invalid!" );
    assert(fChain_ak7 && "AK7 tree invalid!" );
    fChain_ak4->SetBranchStatus("*",0);  // disable all branches
    fChain_ak7->SetBranchStatus("*",0);


    // AK7

    // To begin, read only these 3 variables
    fChain_ak7->SetBranchStatus("EvtHdr_.mRun",1); // run
    fChain_ak7->SetBranchStatus("EvtHdr_.mLumi",1); // lumi
    fChain_ak7->SetBranchStatus("EvtHdr_.mEvent",1); // event

    std::cout << "Mapping AK7 events to AK4 events for data" << std::endl;
    map<Int_t, map<Int_t, map<UInt_t, Long64_t> > > ak7entry;

    Long64_t nentries7 = fChain_ak7->GetEntries();
    for (Long64_t jentry7 = 0; jentry7 < nentries7; jentry7++) {
        fChain_ak7->GetEntry(jentry7);

        // Check for duplicates
        assert(ak7entry[EvtHdr__mRun_ak7][EvtHdr__mLumi_ak7][EvtHdr__mEvent_ak7] == 0);
        ak7entry[EvtHdr__mRun_ak7][EvtHdr__mLumi_ak7][EvtHdr__mEvent_ak7] = jentry7;
    } 

    std::cout << "Found mapping for " << ak7entry.size() << " runs" << std::endl;

    // Enable the remaining variables

    fChain_ak4->SetBranchStatus("PFJets_",1); // ak4_njet
    fChain_ak4->SetBranchStatus("PFJets_.P4_.fCoordinates.f*",1); // Four-momentum
    
    fChain_ak4->SetBranchStatus("PFJets_.tightID_",1); // jet_tightID
    fChain_ak4->SetBranchStatus("PFJets_.area_",1); // jet_area
    fChain_ak4->SetBranchStatus("PFJets_.cor_",1); // jet_jes
    
    // Composition values
    fChain_ak4->SetBranchStatus("PFJets_.chf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nhf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nemf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.cemf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.muf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.hf_*",1);
    fChain_ak4->SetBranchStatus("PFJets_.chm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nhm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.phm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.elm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.mum_",1);
    fChain_ak4->SetBranchStatus("PFJets_.QGtagger_",1);
    fChain_ak4->SetBranchStatus("PFJets_.beta_",1);
    fChain_ak4->SetBranchStatus("PFJets_.betaStar_",1);
    fChain_ak4->SetBranchStatus("PFJets_.hof_",1);

    
    if (isMC) { // MC
        fChain_ak4->SetBranchStatus("GenJets_",1); // ngen
        fChain_ak4->SetBranchStatus("GenJets_.fCoordinates.f*",1);
    }
    
    fChain_ak4->SetBranchStatus("TriggerDecision_",1);
    fChain_ak4->SetBranchStatus("L1Prescale_",1);
    fChain_ak4->SetBranchStatus("HLTPrescale_",1);

    fChain_ak4->SetBranchStatus("EvtHdr_.mRun",1); // run
    fChain_ak4->SetBranchStatus("EvtHdr_.mLumi",1); // lumi
    fChain_ak4->SetBranchStatus("EvtHdr_.mEvent",1); // event
    fChain_ak4->SetBranchStatus("PFMet_.et_",1); // met
    fChain_ak4->SetBranchStatus("PFMet_.sumEt_",1); // sumet
    fChain_ak4->SetBranchStatus("EvtHdr_.mPFRho",1); // rho
    fChain_ak4->SetBranchStatus("EvtHdr_.mPthat",1); // pthat
    fChain_ak4->SetBranchStatus("EvtHdr_.mWeight",1); // mcweight


   	fChain_ak7->SetBranchStatus("PFJets_", 1);
   	fChain_ak7->SetBranchStatus("PFJets_.P4_.fCoordinates.f*", 1);
   	fChain_ak7->SetBranchStatus("PFJets_.cor_", 1);
   	fChain_ak7->SetBranchStatus("PFJets_.area_", 1);

    // Helper variables
    TLorentzVector p4, p4_ak4, p4_ak7, p4gen;
    
    // Total number of events
    Long64_t nentries = fChain_ak4->GetEntries(); 
    std::cout << "Total entries: " << nentries << std::endl;
 
    // DEBUG!!
 	//nentries = 1000000;

    // Convert set into vector
    std::vector<std::string> trg_vec;

    // Process triggers only for data
    if (!isMC) {    

        // Trigger names, common to all events
        assert(TriggerNames);
        auto trg_list = TriggerNames->GetXaxis()->GetLabels();

        // Helper variables, not included in the output
        set<std::string> s;
        for (int i = 0; i != trg_list->GetSize(); ++i ) {

            std::string trg_name(((TObjString*)trg_list->At(i))->GetName());

            // Mapping input trigger indexes to output trigger list
            // "HLT_Jet150_v3" into "HLT_Jet150" 
            s.insert(removeTrgVersion(trg_name));
        }

        trg_vec.assign( s.begin(), s.end() );
        // And sort the vector wrt. the trigger momentum 
        //sort(trg_vec.begin(), trg_vec.end(), compareTrgNames);

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

        fChain_ak4->GetEntry(jentry);

        int i_out = 0;
        for (int i = 0; i != PFJets__; ++i) {

            p4.SetPxPyPzE(  PFJets__P4__fCoordinates_fX[i], PFJets__P4__fCoordinates_fY[i],
                            PFJets__P4__fCoordinates_fZ[i], PFJets__P4__fCoordinates_fT[i]);

        	// pT selection
            Float_t minPt = 15;

        	if (p4.Pt() > minPt) {

	            ak4_pt[i_out] = p4.Pt();
	            ak4_eta[i_out] = p4.Eta();
	            ak4_phi[i_out] = p4.Phi();
	            ak4_E[i_out] = p4.E();


	            ak4_tightID[i_out] = PFJets__tightID_[i];
	            ak4_area[i_out] = PFJets__area_[i];
	            ak4_jes[i_out] = PFJets__cor_[i]; 


	            // Jet composition
	            chf[i_out] 	= PFJets__chf_[i];
			   	nhf[i_out] 	= PFJets__nhf_[i];
			   	phf[i_out] = PFJets__nemf_[i];
			   	elf[i_out] = PFJets__cemf_[i];
			   	muf[i_out] 	= PFJets__muf_[i];
			   	hf_hf[i] 	= PFJets__hf_hf_[i];
			   	hf_phf[i] 	= PFJets__hf_phf_[i];
			   	hf_hm[i] 	= PFJets__hf_hm_[i];
			   	hf_phm[i] 	= PFJets__hf_phm_[i];
			   	chm[i_out] 	= PFJets__chm_[i];
			   	nhm[i_out] 	= PFJets__nhm_[i];
			   	phm[i_out] 	= PFJets__phm_[i];
			   	elm[i_out] 	= PFJets__elm_[i];
			   	mum[i_out] 	= PFJets__mum_[i];

   				qgl[i_out]  = PFJets__QGtagger_[i];
   				beta[i_out]  = PFJets__beta_[i];
   				bstar[i_out]  = PFJets__betaStar_[i];
   				hof[i_out]  = PFJets__hof_[i];


		   		++i_out;
			   	}
        }
        ak4_njet = i_out;

        // Using the entry mapping (AK4 to AK7)

        fChain_ak7->GetEntry(ak7entry[EvtHdr__mRun][EvtHdr__mLumi][EvtHdr__mEvent]);



        // Safety check
        if (EvtHdr__mRun != EvtHdr__mRun_ak7 || 
        	EvtHdr__mEvent != EvtHdr__mEvent_ak7 ||
        	EvtHdr__mLumi != EvtHdr__mLumi_ak7) {
        	
        	assert(false && "Mismatch between AK4 and AK7 events!!");
        	}



        // Keep only four leading jets
		ak7_njet = min(PFJets_ak7__, 4);
        for (int i = 0; i != ak7_njet; ++i) {

            p4_ak7.SetPxPyPzE(  PFJets__P4__fCoordinates_fX_ak7[i], PFJets__P4__fCoordinates_fY_ak7[i],
                                PFJets__P4__fCoordinates_fZ_ak7[i], PFJets__P4__fCoordinates_fT_ak7[i]);

            ak7_pt[i]	= p4_ak7.Pt();   
            ak7_eta[i] 	= p4_ak7.Eta();
            ak7_phi[i] 	= p4_ak7.Phi();
            ak7_E[i] 	= p4_ak7.E();


            ak7_area[i] = PFJets__area_ak7_[i];
            ak7_jes[i] 	= PFJets__cor_ak7_[i]; 

            // Matching AK7 jet to AK4
            ak7_to_ak4[i] = -1;

            // Search AK4 jet with minimum distance to this PFjet   
            float rMin(999);
            for (unsigned int ak4_i = 0; ak4_i != ak4_njet; ++ak4_i) {
                

                // Initialize AK4 jet
                p4_ak4.SetPtEtaPhiE(  ak4_pt[ak4_i],  ak4_eta[ak4_i],
                                      ak4_phi[ak4_i], ak4_E[ak4_i]);  
                // Distance between jets
                double deltaR = p4_ak7.DeltaR(p4_ak4);


                if (deltaR < rMin) {
                    rMin = deltaR;
                    ak7_to_ak4[i] = ak4_i;
                }
            }

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

        // Event identification
        run = EvtHdr__mRun;
        event = EvtHdr__mEvent;
        lumi = EvtHdr__mLumi;


        // MET, SuMET, rho
        met = PFMet__et_;
        sumet = PFMet__sumEt_;
        rho = EvtHdr__mPFRho;

        tree->Fill();
   }

   fout->cd();
   tree->Write();
   fout->Close();
}
