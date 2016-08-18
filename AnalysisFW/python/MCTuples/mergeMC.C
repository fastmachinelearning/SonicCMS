#define mergeMC_cxx
#include "mergeMC.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void mergeMC::Loop()
{
    // Safety
    if (fChain == 0) return;

    // Final tuple file
    TFile *newfile = new TFile("MC_tuples.root","RECREATE");
    
    // Subdirectory
    TDirectory *ydir = newfile->mkdir("ak5ak7");
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

        // Divide cross section by number of events to get total integrated luminosity
        UInt_t const eventNum = fChain->GetTree()->GetEntries(); 
        mcweight /= eventNum; // 'mcweight' already contained the cross section (!)
        
        // Write to the tree
        newtree->Fill();
   }


   newtree->AutoSave();
   delete newfile;
}
