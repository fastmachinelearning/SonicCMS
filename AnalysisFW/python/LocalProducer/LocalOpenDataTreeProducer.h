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
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.
   static const Int_t kMaxGenJets_ = 25;
   static const Int_t kMaxPFJets_ = 25;


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

 //vector<vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > > > HLTObj_;
 //vector<vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > > > L1Obj_;


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
   //vector<double>  PFJets__jecLabels_[kMaxPFJets_];

   Float_t         PFJets__area_[kMaxPFJets_];   //[PFJets__]
   Bool_t          PFJets__tightID_[kMaxPFJets_];   //[PFJets__]

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
   //TBranch        *b_PFJets__jecLabels_;   //!
   
   TBranch        *b_PFJets__area_;   //!
   TBranch        *b_PFJets__tightID_;   //!


   LocalOpenDataTreeProducer(TTree *tree=0);
   virtual ~LocalOpenDataTreeProducer();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef LocalOpenDataTreeProducer_cxx
LocalOpenDataTreeProducer::LocalOpenDataTreeProducer(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.

    if (tree == 0) {
        //const char* filename = "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/MC/ZeroField/Pythia8-CUETM1-Flat-HighStatistics-MagnetOff.root";
        //const char* filename = "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/Data/25ns76X/Ntuples-Data-RunC-ReReco16Dec2015-76X.root";
        const char* filename = "root://eoscms.cern.ch//store/group/phys_smp/Multijet/13TeV/Data/25ns80X/Ntuples-Data-RunB-2016-80X.root";
        

        TFile *f = TFile::Open(filename);
        TDirectory *dir = (TDirectory*)f->Get("ak4");
        dir->GetObject("ProcessedTree", tree);
        dir->GetObject("TriggerNames", TriggerNames);    
    }

   Init(tree);
   Loop();
}

LocalOpenDataTreeProducer::~LocalOpenDataTreeProducer()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t LocalOpenDataTreeProducer::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t LocalOpenDataTreeProducer::LoadTree(Long64_t entry)
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

void LocalOpenDataTreeProducer::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EvtHdr_.mRun", &EvtHdr__mRun, &b_events_EvtHdr__mRun);
   fChain->SetBranchAddress("EvtHdr_.mEvent", &EvtHdr__mEvent, &b_events_EvtHdr__mEvent);
   fChain->SetBranchAddress("EvtHdr_.mLumi", &EvtHdr__mLumi, &b_events_EvtHdr__mLumi);
   fChain->SetBranchAddress("EvtHdr_.mPthat", &EvtHdr__mPthat, &b_events_EvtHdr__mPthat);
   fChain->SetBranchAddress("EvtHdr_.mWeight", &EvtHdr__mWeight, &b_events_EvtHdr__mWeight);

   fChain->SetBranchAddress("EvtHdr_.mPFRho", &EvtHdr__mPFRho, &b_events_EvtHdr__mPFRho);
   fChain->SetBranchAddress("PFMet_.et_", &PFMet__et_, &b_events_PFMet__et_);
   fChain->SetBranchAddress("PFMet_.sumEt_", &PFMet__sumEt_, &b_events_PFMet__sumEt_);

   fChain->SetBranchAddress("TriggerDecision_", &TriggerDecision_, &b_events_TriggerDecision_);
   fChain->SetBranchAddress("L1Prescale_", &L1Prescale_, &b_events_L1Prescale_);
   fChain->SetBranchAddress("HLTPrescale_", &HLTPrescale_, &b_events_HLTPrescale_);

   fChain->SetBranchAddress("GenJets_", &GenJets__, &b_events_GenJets__);
   fChain->SetBranchAddress("GenJets_.fCoordinates.fX", &GenJets__fCoordinates_fX, &b_GenJets__fCoordinates_fX);
   fChain->SetBranchAddress("GenJets_.fCoordinates.fY", &GenJets__fCoordinates_fY, &b_GenJets__fCoordinates_fY);
   fChain->SetBranchAddress("GenJets_.fCoordinates.fZ", &GenJets__fCoordinates_fZ, &b_GenJets__fCoordinates_fZ);
   fChain->SetBranchAddress("GenJets_.fCoordinates.fT", &GenJets__fCoordinates_fT, &b_GenJets__fCoordinates_fT);

   fChain->SetBranchAddress("PFJets_", &PFJets__, &b_events_PFJets__);
   fChain->SetBranchAddress("PFJets_.P4_.fCoordinates.fX", PFJets__P4__fCoordinates_fX, &b_PFJets__P4__fCoordinates_fX);
   fChain->SetBranchAddress("PFJets_.P4_.fCoordinates.fY", PFJets__P4__fCoordinates_fY, &b_PFJets__P4__fCoordinates_fY);
   fChain->SetBranchAddress("PFJets_.P4_.fCoordinates.fZ", PFJets__P4__fCoordinates_fZ, &b_PFJets__P4__fCoordinates_fZ);
   fChain->SetBranchAddress("PFJets_.P4_.fCoordinates.fT", PFJets__P4__fCoordinates_fT, &b_PFJets__P4__fCoordinates_fT);

/*
// Index or four-vector??
   fChain->SetBranchAddress("PFJets_.genP4_.fCoordinates.fX", PFJets__genP4__fCoordinates_fX, &b_PFJets__genP4__fCoordinates_fX);
   fChain->SetBranchAddress("PFJets_.genP4_.fCoordinates.fY", PFJets__genP4__fCoordinates_fY, &b_PFJets__genP4__fCoordinates_fY);
   fChain->SetBranchAddress("PFJets_.genP4_.fCoordinates.fZ", PFJets__genP4__fCoordinates_fZ, &b_PFJets__genP4__fCoordinates_fZ);
   fChain->SetBranchAddress("PFJets_.genP4_.fCoordinates.fT", PFJets__genP4__fCoordinates_fT, &b_PFJets__genP4__fCoordinates_fT);
*/

   fChain->SetBranchAddress("PFJets_.cor_", PFJets__cor_, &b_PFJets__cor_);
//   fChain->SetBranchAddress("PFJets_.jecLabels_", PFJets__jecLabels_, &b_PFJets__jecLabels_);
   fChain->SetBranchAddress("PFJets_.area_", PFJets__area_, &b_PFJets__area_);
   fChain->SetBranchAddress("PFJets_.tightID_", PFJets__tightID_, &b_PFJets__tightID_);

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
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t LocalOpenDataTreeProducer::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef LocalOpenDataTreeProducer_cxx
