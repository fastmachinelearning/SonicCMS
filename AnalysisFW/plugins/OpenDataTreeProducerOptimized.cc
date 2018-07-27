

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
#include <time.h>


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

// #include "fastjet/contrib/SoftDrop.hh"

OpenDataTreeProducerOptimized::OpenDataTreeProducerOptimized(edm::ParameterSet const &cfg) :
  JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
  JetTok_(consumes<edm::View<pat::Jet>>(JetTag_))
{
}

void OpenDataTreeProducerOptimized::beginJob() {
        
    // load the graph 
    std::cout << "[OpenDataTreeProducerOptimized::beginRun] loading the .pb files..." << std::endl;
    tensorflow::setLogging();
    graphDef_ = tensorflow::loadGraphDef("resnet50_classifier.pb");
    // graphDefClassifier_ = tensorflow::loadGraphDef("resnet50_classifier.pb");
    
    time_t my_time = time(NULL);
    printf("%s", ctime(&my_time));        
}

void OpenDataTreeProducerOptimized::endJob() {
}


void OpenDataTreeProducerOptimized::beginRun(edm::Run const &iRun,edm::EventSetup const &iSetup) { 
}


void OpenDataTreeProducerOptimized::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {
  
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

    // convert image to tensor
    tensorflow::Tensor inputImage(tensorflow::DT_FLOAT, { 1, 224, 224, 3 });
    // std::cout << "inputImage shape = " << inputImage.tensor<float,(4)>() << std::endl;
    auto input_map = inputImage.tensor<float, 4>();
    for (int itf = 0; itf < 224; itf++){
      for (int jtf = 0; jtf < 224; jtf++){
        input_map(0,itf,jtf,0) = image2D[itf][jtf];
        input_map(0,itf,jtf,1) = image2D[itf][jtf];
        input_map(0,itf,jtf,2) = image2D[itf][jtf];
      }
    }

    // (?, 1, 1, 2048)
    tensorflow::Tensor inputClassifer(tensorflow::DT_FLOAT, { 1, 1, 1, 2048 });
    auto input_map_classifier = inputClassifer.tensor<float,4>();
    for (int itf = 0; itf < 2048; itf++){
      input_map_classifier(0,0,0,itf) = (float) itf * 0.1;
    }

    // Tensorflow part
    std::cout << "creating session..." << std::endl;
    tensorflow::Session* session = tensorflow::createSession(graphDef_);
    std::cout << "Now can run..." << std::endl;

    time_t my_time2 = time(NULL);
    printf("%s", ctime(&my_time2));

    std::vector<tensorflow::Tensor> outputs;
    // tensorflow::Status status = session->Run( {{"prefix/InputImage:0",inputImage}}, { "prefix/resnet_v1_50/pool5:0" }, {}, &outputs);
    tensorflow::Status status = session->Run( {{"prefix/Input:0",inputClassifer}}, { "prefix/resnet_v1_50/logits/Softmax:0" }, {}, &outputs);
    std::cout << "Does this work??" << std::endl;

    time_t my_time3 = time(NULL);
    printf("%s", ctime(&my_time3));

    std::cout << "close the session..." << std::endl;
    tensorflow::closeSession(session);

}



void OpenDataTreeProducerOptimized::endRun(edm::Run const &iRun, edm::EventSetup const &iSetup) {

}

OpenDataTreeProducerOptimized::~OpenDataTreeProducerOptimized() {
}


DEFINE_FWK_MODULE(OpenDataTreeProducerOptimized);
