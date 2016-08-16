//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jun 23 18:09:40 2016 by ROOT version 6.04/14
// from TTree ProcessedTree/ProcessedTree
// found on file: ProcessedTree_data.root
//////////////////////////////////////////////////////////

#ifndef LocalOpenDataTreeProducer_h
#define LocalOpenDataTreeProducer_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "Math/GenVector/PxPyPzE4D.h"

class LocalOpenDataTreeProducer {
public :
   TTree          *fChain_ak4;   //!pointer to the analyzed TTree or TChain
   TTree          *fChain_ak7;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.
   static const Int_t kMaxGenJets_ = 64;
   static const Int_t kMaxPFJets_ = 64;


   // Declaration of leaf types
   TH1F*           TriggerNames;

   Int_t           EvtHdr__mRun;
   Int_t           EvtHdr__mEvent;
   Int_t           EvtHdr__mLumi;

   Float_t         EvtHdr__mPthat;
   Float_t         EvtHdr__mWeight;
   Float_t         EvtHdr__mPFRho;

   Float_t         PFMet__et_;
   Float_t         PFMet__sumEt_;

   vector<int>     TriggerDecision_;
   vector<int>     L1Prescale_;
   vector<int>     HLTPrescale_;

   Int_t           GenJets__;
   Double_t        GenJets__fCoordinates_fX[kMaxGenJets_];   //[GenJets__]
   Double_t        GenJets__fCoordinates_fY[kMaxGenJets_];   //[GenJets__]
   Double_t        GenJets__fCoordinates_fZ[kMaxGenJets_];   //[GenJets__]
   Double_t        GenJets__fCoordinates_fT[kMaxGenJets_];   //[GenJets__]

   Int_t           PFJets__;
   Double_t        PFJets__P4__fCoordinates_fX[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fY[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fZ[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fT[kMaxPFJets_];   //[PFJets__]
   
   /*
   Double_t        PFJets__genP4__fCoordinates_fX[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__genP4__fCoordinates_fY[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__genP4__fCoordinates_fZ[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__genP4__fCoordinates_fT[kMaxPFJets_];   //[PFJets__]
   */
   
   Float_t         PFJets__cor_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__area_[kMaxPFJets_];   //[PFJets__]
   Bool_t          PFJets__tightID_[kMaxPFJets_];   //[PFJets__]

   Float_t         PFJets__chf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__nhf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__nemf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__cemf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__muf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__hf_hf_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__hf_phf_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__hf_hm_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__hf_phm_[kMaxPFJets_];   //[PFJets__]*/
   Int_t           PFJets__chm_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__nhm_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__phm_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__elm_[kMaxPFJets_];   //[PFJets__]
   Int_t           PFJets__mum_[kMaxPFJets_];   //[PFJets__]

   Float_t         PFJets__QGtagger_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__beta_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__betaStar_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__hof_[kMaxPFJets_];   //[PFJets__]

   Int_t           EvtHdr__mRun_ak7;
   Int_t           EvtHdr__mEvent_ak7;
   Int_t           EvtHdr__mLumi_ak7;

   Int_t           PFJets_ak7__;
   Double_t        PFJets__P4__fCoordinates_fX_ak7[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fY_ak7[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fZ_ak7[kMaxPFJets_];   //[PFJets__]
   Double_t        PFJets__P4__fCoordinates_fT_ak7[kMaxPFJets_];   //[PFJets__]
      
   Float_t         PFJets__cor_ak7_[kMaxPFJets_];   //[PFJets__]
   Float_t         PFJets__area_ak7_[kMaxPFJets_];   //[PFJets__]


   // List of branches
   TBranch        *b_events_EvtHdr__mRun;   //!
   TBranch        *b_events_EvtHdr__mEvent;   //!
   TBranch        *b_events_EvtHdr__mLumi;   //!

   TBranch        *b_events_EvtHdr__mPthat;   //!
   TBranch        *b_events_EvtHdr__mWeight;   //!

   TBranch        *b_events_EvtHdr__mPFRho;   //!
   TBranch        *b_events_PFMet__et_;   //!
   TBranch        *b_events_PFMet__sumEt_;   //!

   TBranch        *b_events_TriggerDecision_;   //!
   TBranch        *b_events_L1Prescale_;   //!
   TBranch        *b_events_HLTPrescale_;   //!

   TBranch        *b_events_GenJets__;   //!
   TBranch        *b_GenJets__fCoordinates_fX;   //!
   TBranch        *b_GenJets__fCoordinates_fY;   //!
   TBranch        *b_GenJets__fCoordinates_fZ;   //!
   TBranch        *b_GenJets__fCoordinates_fT;   //!

   TBranch        *b_events_PFJets__;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fX;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fY;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fZ;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fT;   //!
   /*
   TBranch        *b_PFJets__genP4__fCoordinates_fX;   //!
   TBranch        *b_PFJets__genP4__fCoordinates_fY;   //!
   TBranch        *b_PFJets__genP4__fCoordinates_fZ;   //!
   TBranch        *b_PFJets__genP4__fCoordinates_fT;   //!
   */
   TBranch        *b_PFJets__cor_;   //!
   
   TBranch        *b_PFJets__area_;   //!
   TBranch        *b_PFJets__tightID_;   //!


   TBranch        *b_PFJets__chf_;   //!
   TBranch        *b_PFJets__nhf_;   //!
   TBranch        *b_PFJets__nemf_;   //!
   TBranch        *b_PFJets__cemf_;   //!
   TBranch        *b_PFJets__muf_;   //!
   TBranch        *b_PFJets__hf_hf_;   //!
   TBranch        *b_PFJets__hf_phf_;   //!
   TBranch        *b_PFJets__hf_hm_;   //!
   TBranch        *b_PFJets__hf_phm_;   //!
   TBranch        *b_PFJets__chm_;   //!
   TBranch        *b_PFJets__nhm_;   //!
   TBranch        *b_PFJets__phm_;   //!
   TBranch        *b_PFJets__elm_;   //!
   TBranch        *b_PFJets__mum_;   //!

   TBranch        *b_PFJets__QGtagger_;   //!
   TBranch        *b_PFJets__beta_;   //!
   TBranch        *b_PFJets__betaStar_;   //!
   TBranch        *b_PFJets__hof_;   //!

   TBranch        *b_events_EvtHdr__mRun_ak7;   //!
   TBranch        *b_events_EvtHdr__mEvent_ak7;   //!
   TBranch        *b_events_EvtHdr__mLumi_ak7;   //!

   TBranch        *b_events_PFJets_ak7__;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fX_ak7;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fY_ak7;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fZ_ak7;   //!
   TBranch        *b_PFJets__P4__fCoordinates_fT_ak7;   //!

   TBranch        *b_PFJets__cor_ak7_;  
   TBranch        *b_PFJets__area_ak7_;   //!


   LocalOpenDataTreeProducer(TTree *tree_ak4 = 0, TTree *tree_ak7 = 0);
   virtual ~LocalOpenDataTreeProducer();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree_ak4 = 0, TTree *tree_ak7 = 0);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef LocalOpenDataTreeProducer_cxx
LocalOpenDataTreeProducer::LocalOpenDataTreeProducer(TTree *tree_ak4, TTree *tree_ak7) : fChain_ak4(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.

   const std::string treeName = "ProcessedTree";
   const int numFiles = 3;

   // Here are the input files!
   const char *fileNames[numFiles] = { "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/Data/2016/Ntuples-Data-2016-RunC-v2-part1.root",
                                       "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/Data/2016/Ntuples-Data-2016-RunC-v2-part2.root",
                                       "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/Data/2016/Ntuples-Data-2016-RunC-v2-part3.root",
                                    };

   // AK4 and AK7 trees
   TChain *chain_ak4 = new TChain(("ak4/" + treeName).c_str());
   TChain *chain_ak7 = new TChain(("ak7/" + treeName).c_str());

   for (int i = 0; i < numFiles; ++i) {
      chain_ak4->Add(fileNames[i]);
      chain_ak7->Add(fileNames[i]);
   }

   // Retrieve trigger names
   TFile *f = TFile::Open(fileNames[0]);
   TDirectory *dir = (TDirectory*)f->Get("ak4");
   dir->GetObject("TriggerNames", TriggerNames);    

   // Process the trees
   tree_ak4 = chain_ak4;
   tree_ak7 = chain_ak7;
   Init(tree_ak4, tree_ak7);
   Loop();
}

LocalOpenDataTreeProducer::~LocalOpenDataTreeProducer()
{

}

Int_t LocalOpenDataTreeProducer::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain_ak4) return 0;
   return fChain_ak4->GetEntry(entry);
}

Long64_t LocalOpenDataTreeProducer::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain_ak4) return -5;
   Long64_t centry = fChain_ak4->LoadTree(entry);

   if (centry < 0) return centry;
   if (fChain_ak4->GetTreeNumber() != fCurrent) {
      fCurrent = fChain_ak4->GetTreeNumber();
      Notify();
   }
   return centry;
}

void LocalOpenDataTreeProducer::Init(TTree *tree_ak4, TTree *tree_ak7)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree_ak4) return;
   fChain_ak4 = tree_ak4;
   fCurrent = -1;
   fChain_ak4->SetMakeClass(1);

   fChain_ak4->SetBranchAddress("EvtHdr_.mRun", &EvtHdr__mRun, &b_events_EvtHdr__mRun);
   fChain_ak4->SetBranchAddress("EvtHdr_.mEvent", &EvtHdr__mEvent, &b_events_EvtHdr__mEvent);
   fChain_ak4->SetBranchAddress("EvtHdr_.mLumi", &EvtHdr__mLumi, &b_events_EvtHdr__mLumi);
   fChain_ak4->SetBranchAddress("EvtHdr_.mPthat", &EvtHdr__mPthat, &b_events_EvtHdr__mPthat);
   fChain_ak4->SetBranchAddress("EvtHdr_.mWeight", &EvtHdr__mWeight, &b_events_EvtHdr__mWeight);

   fChain_ak4->SetBranchAddress("EvtHdr_.mPFRho", &EvtHdr__mPFRho, &b_events_EvtHdr__mPFRho);
   fChain_ak4->SetBranchAddress("PFMet_.et_", &PFMet__et_, &b_events_PFMet__et_);
   fChain_ak4->SetBranchAddress("PFMet_.sumEt_", &PFMet__sumEt_, &b_events_PFMet__sumEt_);

   fChain_ak4->SetBranchAddress("TriggerDecision_", &TriggerDecision_, &b_events_TriggerDecision_);
   fChain_ak4->SetBranchAddress("L1Prescale_", &L1Prescale_, &b_events_L1Prescale_);
   fChain_ak4->SetBranchAddress("HLTPrescale_", &HLTPrescale_, &b_events_HLTPrescale_);
   /* MC
   fChain_ak4->SetBranchAddress("GenJets_", &GenJets__, &b_events_GenJets__);
   fChain_ak4->SetBranchAddress("GenJets_.fCoordinates.fX", GenJets__fCoordinates_fX, &b_GenJets__fCoordinates_fX);
   fChain_ak4->SetBranchAddress("GenJets_.fCoordinates.fY", GenJets__fCoordinates_fY, &b_GenJets__fCoordinates_fY);
   fChain_ak4->SetBranchAddress("GenJets_.fCoordinates.fZ", GenJets__fCoordinates_fZ, &b_GenJets__fCoordinates_fZ);
   fChain_ak4->SetBranchAddress("GenJets_.fCoordinates.fT", GenJets__fCoordinates_fT, &b_GenJets__fCoordinates_fT);
   */
   fChain_ak4->SetBranchAddress("PFJets_", &PFJets__, &b_events_PFJets__);
   fChain_ak4->SetBranchAddress("PFJets_.P4_.fCoordinates.fX", PFJets__P4__fCoordinates_fX, &b_PFJets__P4__fCoordinates_fX);
   fChain_ak4->SetBranchAddress("PFJets_.P4_.fCoordinates.fY", PFJets__P4__fCoordinates_fY, &b_PFJets__P4__fCoordinates_fY);
   fChain_ak4->SetBranchAddress("PFJets_.P4_.fCoordinates.fZ", PFJets__P4__fCoordinates_fZ, &b_PFJets__P4__fCoordinates_fZ);
   fChain_ak4->SetBranchAddress("PFJets_.P4_.fCoordinates.fT", PFJets__P4__fCoordinates_fT, &b_PFJets__P4__fCoordinates_fT);
/*
// MC : Index or four-vector??
   fChain_ak4->SetBranchAddress("PFJets_.genP4_.fCoordinates.fX", PFJets__genP4__fCoordinates_fX, &b_PFJets__genP4__fCoordinates_fX);
   fChain_ak4->SetBranchAddress("PFJets_.genP4_.fCoordinates.fY", PFJets__genP4__fCoordinates_fY, &b_PFJets__genP4__fCoordinates_fY);
   fChain_ak4->SetBranchAddress("PFJets_.genP4_.fCoordinates.fZ", PFJets__genP4__fCoordinates_fZ, &b_PFJets__genP4__fCoordinates_fZ);
   fChain_ak4->SetBranchAddress("PFJets_.genP4_.fCoordinates.fT", PFJets__genP4__fCoordinates_fT, &b_PFJets__genP4__fCoordinates_fT);
*/

   fChain_ak4->SetBranchAddress("PFJets_.cor_", PFJets__cor_, &b_PFJets__cor_);
   fChain_ak4->SetBranchAddress("PFJets_.area_", PFJets__area_, &b_PFJets__area_);
   fChain_ak4->SetBranchAddress("PFJets_.tightID_", PFJets__tightID_, &b_PFJets__tightID_);

   fChain_ak4->SetBranchAddress("PFJets_.chf_", PFJets__chf_, &b_PFJets__chf_);
   fChain_ak4->SetBranchAddress("PFJets_.nhf_", PFJets__nhf_, &b_PFJets__nhf_);
   fChain_ak4->SetBranchAddress("PFJets_.nemf_", PFJets__nemf_, &b_PFJets__nemf_);
   fChain_ak4->SetBranchAddress("PFJets_.cemf_", PFJets__cemf_, &b_PFJets__cemf_);
   fChain_ak4->SetBranchAddress("PFJets_.muf_", PFJets__muf_, &b_PFJets__muf_);
   fChain_ak4->SetBranchAddress("PFJets_.hf_hf_", PFJets__hf_hf_, &b_PFJets__hf_hf_);
   fChain_ak4->SetBranchAddress("PFJets_.hf_phf_", PFJets__hf_phf_, &b_PFJets__hf_phf_);
   fChain_ak4->SetBranchAddress("PFJets_.hf_hm_", PFJets__hf_hm_, &b_PFJets__hf_hm_);
   fChain_ak4->SetBranchAddress("PFJets_.hf_phm_", PFJets__hf_phm_, &b_PFJets__hf_phm_);
   fChain_ak4->SetBranchAddress("PFJets_.chm_", PFJets__chm_, &b_PFJets__chm_);
   fChain_ak4->SetBranchAddress("PFJets_.nhm_", PFJets__nhm_, &b_PFJets__nhm_);
   fChain_ak4->SetBranchAddress("PFJets_.phm_", PFJets__phm_, &b_PFJets__phm_);
   fChain_ak4->SetBranchAddress("PFJets_.elm_", PFJets__elm_, &b_PFJets__elm_);
   fChain_ak4->SetBranchAddress("PFJets_.mum_", PFJets__mum_, &b_PFJets__mum_);


   fChain_ak4->SetBranchAddress("PFJets_.QGtagger_", PFJets__QGtagger_, &b_PFJets__QGtagger_);
   fChain_ak4->SetBranchAddress("PFJets_.beta_", PFJets__beta_, &b_PFJets__beta_);
   fChain_ak4->SetBranchAddress("PFJets_.betaStar_", PFJets__betaStar_, &b_PFJets__betaStar_);
   fChain_ak4->SetBranchAddress("PFJets_.hof_", PFJets__hof_, &b_PFJets__hof_);
   
   fChain_ak7 = tree_ak7;
   if (fChain_ak7) fChain_ak7->SetMakeClass(1);


   fChain_ak7->SetBranchAddress("EvtHdr_.mRun", &EvtHdr__mRun_ak7, &b_events_EvtHdr__mRun_ak7);
   fChain_ak7->SetBranchAddress("EvtHdr_.mEvent", &EvtHdr__mEvent_ak7, &b_events_EvtHdr__mEvent_ak7);
   fChain_ak7->SetBranchAddress("EvtHdr_.mLumi", &EvtHdr__mLumi_ak7, &b_events_EvtHdr__mLumi_ak7);

   fChain_ak7->SetBranchAddress("PFJets_", &PFJets_ak7__, &b_events_PFJets_ak7__);
   fChain_ak7->SetBranchAddress("PFJets_.P4_.fCoordinates.fX", PFJets__P4__fCoordinates_fX_ak7, &b_PFJets__P4__fCoordinates_fX_ak7);
   fChain_ak7->SetBranchAddress("PFJets_.P4_.fCoordinates.fY", PFJets__P4__fCoordinates_fY_ak7, &b_PFJets__P4__fCoordinates_fY_ak7);
   fChain_ak7->SetBranchAddress("PFJets_.P4_.fCoordinates.fZ", PFJets__P4__fCoordinates_fZ_ak7, &b_PFJets__P4__fCoordinates_fZ_ak7);
   fChain_ak7->SetBranchAddress("PFJets_.P4_.fCoordinates.fT", PFJets__P4__fCoordinates_fT_ak7, &b_PFJets__P4__fCoordinates_fT_ak7);

   fChain_ak7->SetBranchAddress("PFJets_.cor_", PFJets__cor_ak7_, &b_PFJets__cor_ak7_);
   fChain_ak7->SetBranchAddress("PFJets_.area_", PFJets__area_ak7_, &b_PFJets__area_ak7_);
   

   Notify();
}

Bool_t LocalOpenDataTreeProducer::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void LocalOpenDataTreeProducer::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain_ak4) return;
   fChain_ak4->Show(entry);
}
Int_t LocalOpenDataTreeProducer::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef LocalOpenDataTreeProducer_cxx
