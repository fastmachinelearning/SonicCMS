//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Aug 18 11:11:29 2016 by ROOT version 5.32/00
// from TTree OpenDataTree/OpenDataTree
// found on file: OpenDataTree_mc.root
//////////////////////////////////////////////////////////

#ifndef allVar_h
#define allVar_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <vector>

// Fixed size dimensions of array or collections stored in the TTree if any.

class allVar {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   UInt_t          njet;
   Float_t         jet_pt[5];   //[njet]
   Float_t         jet_eta[5];   //[njet]
   Float_t         jet_phi[5];   //[njet]
   Float_t         jet_E[5];   //[njet]
   Bool_t          jet_tightID[5];   //[njet]
   Float_t         jet_area[5];   //[njet]
   Float_t         jet_jes[5];   //[njet]
   Int_t           jet_igen[5];   //[njet]
   UInt_t          njet_ak7;
   Float_t         jet_pt_ak7[4];   //[njet_ak7]
   Float_t         jet_eta_ak7[4];   //[njet_ak7]
   Float_t         jet_phi_ak7[4];   //[njet_ak7]
   Float_t         jet_E_ak7[4];   //[njet_ak7]
   Float_t         jet_area_ak7[4];   //[njet_ak7]
   Float_t         jet_jes_ak7[4];   //[njet_ak7]
   Int_t           ak7_to_ak5[4];   //[njet_ak7]
   UInt_t          ngen;
   Float_t         gen_pt[5];   //[ngen]
   Float_t         gen_eta[5];   //[ngen]
   Float_t         gen_phi[5];   //[ngen]
   Float_t         gen_E[5];   //[ngen]
   UInt_t          run;
   UInt_t          lumi;
   ULong64_t       event;
   UInt_t          ntrg;
   Bool_t          triggers[6];   //[ntrg]
   vector<string>  *triggernames;
   UInt_t          prescales[6];   //[ntrg]
   Float_t         met;
   Float_t         sumet;
   Float_t         rho;
   Float_t         pthat;
   Float_t         mcweight;
   Float_t         chf[5];   //[njet]
   Float_t         nhf[5];   //[njet]
   Float_t         phf[5];   //[njet]
   Float_t         elf[5];   //[njet]
   Float_t         muf[5];   //[njet]
   Float_t         hf_hf[5];   //[njet]
   Float_t         hf_phf[5];   //[njet]
   UInt_t          hf_hm[5];   //[njet]
   UInt_t          hf_phm[5];   //[njet]
   UInt_t          chm[5];   //[njet]
   UInt_t          nhm[5];   //[njet]
   UInt_t          phm[5];   //[njet]
   UInt_t          elm[5];   //[njet]
   UInt_t          mum[5];   //[njet]
   Float_t         beta[5];   //[njet]
   Float_t         bstar[5];   //[njet]

   // List of branches
   TBranch        *b_njet;   //!
   TBranch        *b_jet_pt;   //!
   TBranch        *b_jet_eta;   //!
   TBranch        *b_jet_phi;   //!
   TBranch        *b_jet_E;   //!
   TBranch        *b_jet_tightID;   //!
   TBranch        *b_jet_area;   //!
   TBranch        *b_jet_jes;   //!
   TBranch        *b_jet_igen;   //!
   TBranch        *b_njet_ak7;   //!
   TBranch        *b_jet_pt_ak7;   //!
   TBranch        *b_jet_eta_ak7;   //!
   TBranch        *b_jet_phi_ak7;   //!
   TBranch        *b_jet_E_ak7;   //!
   TBranch        *b_jet_area_ak7;   //!
   TBranch        *b_jet_jes_ak7;   //!
   TBranch        *b_ak7_to_ak5;   //!
   TBranch        *b_ngen;   //!
   TBranch        *b_gen_pt;   //!
   TBranch        *b_gen_eta;   //!
   TBranch        *b_gen_phi;   //!
   TBranch        *b_gen_E;   //!
   TBranch        *b_run;   //!
   TBranch        *b_lumi;   //!
   TBranch        *b_event;   //!
   TBranch        *b_ntrg;   //!
   TBranch        *b_triggers;   //!
   TBranch        *b_triggernames;   //!
   TBranch        *b_prescales;   //!
   TBranch        *b_met;   //!
   TBranch        *b_sumet;   //!
   TBranch        *b_rho;   //!
   TBranch        *b_pthat;   //!
   TBranch        *b_mcweight;   //!
   TBranch        *b_chf;   //!
   TBranch        *b_nhf;   //!
   TBranch        *b_phf;   //!
   TBranch        *b_elf;   //!
   TBranch        *b_muf;   //!
   TBranch        *b_hf_hf;   //!
   TBranch        *b_hf_phf;   //!
   TBranch        *b_hf_hm;   //!
   TBranch        *b_hf_phm;   //!
   TBranch        *b_chm;   //!
   TBranch        *b_nhm;   //!
   TBranch        *b_phm;   //!
   TBranch        *b_elm;   //!
   TBranch        *b_mum;   //!
   TBranch        *b_beta;   //!
   TBranch        *b_bstar;   //!

   allVar(TTree *tree=0);
   virtual ~allVar();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef allVar_cxx
allVar::allVar(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("OpenDataTree_mc.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("OpenDataTree_mc.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("OpenDataTree_mc.root:/ak5ak7");
      dir->GetObject("OpenDataTree",tree);

   }
   Init(tree);
}

allVar::~allVar()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t allVar::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t allVar::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void allVar::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   triggernames = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("njet", &njet, &b_njet);
   fChain->SetBranchAddress("jet_pt", jet_pt, &b_jet_pt);
   fChain->SetBranchAddress("jet_eta", jet_eta, &b_jet_eta);
   fChain->SetBranchAddress("jet_phi", jet_phi, &b_jet_phi);
   fChain->SetBranchAddress("jet_E", jet_E, &b_jet_E);
   fChain->SetBranchAddress("jet_tightID", jet_tightID, &b_jet_tightID);
   fChain->SetBranchAddress("jet_area", jet_area, &b_jet_area);
   fChain->SetBranchAddress("jet_jes", jet_jes, &b_jet_jes);
   fChain->SetBranchAddress("jet_igen", jet_igen, &b_jet_igen);
   fChain->SetBranchAddress("njet_ak7", &njet_ak7, &b_njet_ak7);
   fChain->SetBranchAddress("jet_pt_ak7", jet_pt_ak7, &b_jet_pt_ak7);
   fChain->SetBranchAddress("jet_eta_ak7", jet_eta_ak7, &b_jet_eta_ak7);
   fChain->SetBranchAddress("jet_phi_ak7", jet_phi_ak7, &b_jet_phi_ak7);
   fChain->SetBranchAddress("jet_E_ak7", jet_E_ak7, &b_jet_E_ak7);
   fChain->SetBranchAddress("jet_area_ak7", jet_area_ak7, &b_jet_area_ak7);
   fChain->SetBranchAddress("jet_jes_ak7", jet_jes_ak7, &b_jet_jes_ak7);
   fChain->SetBranchAddress("ak7_to_ak5", ak7_to_ak5, &b_ak7_to_ak5);
   fChain->SetBranchAddress("ngen", &ngen, &b_ngen);
   fChain->SetBranchAddress("gen_pt", gen_pt, &b_gen_pt);
   fChain->SetBranchAddress("gen_eta", gen_eta, &b_gen_eta);
   fChain->SetBranchAddress("gen_phi", gen_phi, &b_gen_phi);
   fChain->SetBranchAddress("gen_E", gen_E, &b_gen_E);
   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("lumi", &lumi, &b_lumi);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("ntrg", &ntrg, &b_ntrg);
   fChain->SetBranchAddress("triggers", triggers, &b_triggers);
   fChain->SetBranchAddress("triggernames", &triggernames, &b_triggernames);
   fChain->SetBranchAddress("prescales", prescales, &b_prescales);
   fChain->SetBranchAddress("met", &met, &b_met);
   fChain->SetBranchAddress("sumet", &sumet, &b_sumet);
   fChain->SetBranchAddress("rho", &rho, &b_rho);
   fChain->SetBranchAddress("pthat", &pthat, &b_pthat);
   fChain->SetBranchAddress("mcweight", &mcweight, &b_mcweight);
   fChain->SetBranchAddress("chf", chf, &b_chf);
   fChain->SetBranchAddress("nhf", nhf, &b_nhf);
   fChain->SetBranchAddress("phf", phf, &b_phf);
   fChain->SetBranchAddress("elf", elf, &b_elf);
   fChain->SetBranchAddress("muf", muf, &b_muf);
   fChain->SetBranchAddress("hf_hf", hf_hf, &b_hf_hf);
   fChain->SetBranchAddress("hf_phf", hf_phf, &b_hf_phf);
   fChain->SetBranchAddress("hf_hm", hf_hm, &b_hf_hm);
   fChain->SetBranchAddress("hf_phm", hf_phm, &b_hf_phm);
   fChain->SetBranchAddress("chm", chm, &b_chm);
   fChain->SetBranchAddress("nhm", nhm, &b_nhm);
   fChain->SetBranchAddress("phm", phm, &b_phm);
   fChain->SetBranchAddress("elm", elm, &b_elm);
   fChain->SetBranchAddress("mum", mum, &b_mum);
   fChain->SetBranchAddress("beta", beta, &b_beta);
   fChain->SetBranchAddress("bstar", bstar, &b_bstar);
   Notify();
}

Bool_t allVar::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void allVar::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t allVar::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef allVar_cxx
