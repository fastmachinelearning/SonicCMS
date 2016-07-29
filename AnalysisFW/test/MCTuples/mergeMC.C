#define mergeMC_cxx
#include "mergeMC.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void mergeMC::Loop()
{
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch

    if (fChain == 0) return;

    // File for merged trees
    TFile *newfile = new TFile("MC_tuples.root","RECREATE");
    
    // Subdirectory
    TDirectory *ydir = newfile->mkdir("ak7");
    assert(ydir);
    ydir->cd();

    // Set branches addresses of the merged tree (same as the old tree)  
    TTree *newtree = fChain->CloneTree(0);

    // Total number of events (all files combined)
    Long64_t nentries = fChain->GetEntriesFast();
    
    // Event loop
    for (Long64_t jentry=0; jentry < nentries;jentry++) {
        
        // Choose the right tree to read 
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        
        // Read values to the variables
        fChain->GetEntry(jentry);

        // Divide cross section by number of events to get total luminosity
        UInt_t const eventNum = fChain->GetTree()->GetEntries(); 
        mcweight /= eventNum; // 'mcweight' contained the cross section (!)
        
        // Write to the tree
        newtree->Fill();
   }


   newtree->AutoSave();
   delete newfile;
}
