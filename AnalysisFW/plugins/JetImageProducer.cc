// Forked from SMPJ Analysis Framework
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW
// https://github.com/cms-smpj/SMPJ/tree/v1.0

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <functional>
#include <vector>
#include <chrono>
#include <map>

#include "Jet2011/AnalysisFW/interface/TFClient.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/graph/default_device.h"

class JetImageProducer : public edm::global::EDProducer<>
{
  public:

    explicit JetImageProducer(edm::ParameterSet const& cfg);
    void produce(edm::StreamID, edm::Event& iEvent, edm::EventSetup const& iSetup) const;
    ~JetImageProducer() override;

  private:
    void loadModel();
	void findTopN(const tensorflow::Tensor& scores, unsigned n=5) const;
    tensorflow::Tensor createImage(const edm::View<pat::Jet>& jets, const edm::Event& iEvent) const;
    std::vector<tensorflow::Tensor> runFeaturizer(const tensorflow::Tensor& input) const;
    tensorflow::Tensor createFeatureList(const tensorflow::Tensor& input) const;
    std::vector<tensorflow::Tensor> runClassifier(const tensorflow::Tensor& input) const;

    edm::InputTag JetTag_;
    edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
    tensorflow::GraphDef* graphDefFeaturizer_;
    tensorflow::GraphDef* graphDefClassifier_;
    TFClient client_;
    bool saveImage_, remote_;
	unsigned topN_;
	std::vector<std::string> imageList_;
};

JetImageProducer::JetImageProducer(edm::ParameterSet const &cfg) :
  JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
  JetTok_(consumes<edm::View<pat::Jet>>(JetTag_)),
  saveImage_(cfg.getParameter<bool>("saveImage")),
  topN_(cfg.getParameter<unsigned>("topN"))
{
    if(cfg.exists("ServerParams")){
        //in remote version, model is already loaded on the server
        remote_ = true;
        const auto& scfg = cfg.getParameter<edm::ParameterSet>("ServerParams");
        client_.setSession(scfg.getParameter<std::string>("address"),scfg.getParameter<int>("port"),scfg.getParameter<unsigned>("timeout"));
        edm::LogInfo("JetImageProducer") << "Connected to remote server";
    }
    else {
        remote_ = false;
        loadModel();
    }

	//load score list
	std::ifstream ifile("imagenet_classes.txt");
	if(ifile.is_open()){
		std::string line;
		while(std::getline(ifile,line)){
			imageList_.push_back(line);
		}
	}
}

void JetImageProducer::loadModel(){
    // load the graph 
    std::stringstream msg;
    msg << "[loadModel] Loading the .pb files...\n";
    tensorflow::setLogging();

    graphDefFeaturizer_ = tensorflow::loadGraphDef("resnet50.pb");
    msg << "featurizer node size = " << graphDefFeaturizer_->node_size() << "\n";
    // Don't print this out -- it's humongous
    // for (int i = 0; i < graphDefFeaturizer_->node_size(); i++) {
    //   std::cout << graphDefFeaturizer_->node(i).name() << std::endl;
    // }
    auto shape0F = graphDefFeaturizer_->node().Get(0).attr().at("shape").shape();
    msg << "featurizer shape0 size = " << shape0F.dim_size() << "\n";
    for (int i = 0; i < shape0F.dim_size(); i++) {
      msg << shape0F.dim(i).size() << "\n";
    }

    // ReadBinaryProto(tensorflow::Env::Default(), "resnet50_classifier.pb", &graphDef_);
    graphDefClassifier_ = tensorflow::loadGraphDef("resnet50_classifier.pb");
    msg << "classifier node size = " << graphDefClassifier_->node_size() << "\n";
    for (int i = 0; i < graphDefClassifier_->node_size(); i++) {
      msg << graphDefClassifier_->node(i).name() << "\n";
    }
    auto shape0C = graphDefClassifier_->node().Get(0).attr().at("shape").shape();
    msg << "classifier shape0 size = " << shape0C.dim_size() << "\n";
    for (int i = 0; i < shape0C.dim_size(); i++) {
      msg << shape0C.dim(i).size() << "\n";
    }
    edm::LogInfo("JetImageProducer") << msg.str();
    
    // apparently the last node does not have a shape, so this is all commented out
    // auto shapeN = graphDef_->node().Get(graphDef_->node_size()-1).attr().at("shape").shape();    
    // std::cout << "shapeN size = " << shapeN.dim_size() << std::endl;
    // for (int i = 0; i < shapeN.dim_size(); i++) {
    //   std::cout << shapeN.dim(i).size() << std::endl;
    // }
}

void JetImageProducer::findTopN(const tensorflow::Tensor& scores, unsigned n) const {
	auto score_list = scores.flat<float>();
	auto dim = score_list.dimensions()[0];

	//match score to type by index, then put in largest-first map
	std::map<float,std::string,std::greater<float>> score_map;
	for(unsigned i = 0; i < std::min((unsigned)dim,(unsigned)imageList_.size()); ++i){
		score_map.emplace(score_list(i),imageList_[i]);
	}

	//get top n
	std::stringstream msg;
	msg << "Scores:\n";
	unsigned counter = 0;
	for(const auto& item: score_map){
		msg << item.second << " : " << item.first << "\n";
		++counter;
		if(counter>=n) break;
	}
	edm::LogInfo("JetImageProducer") << msg.str();
}

tensorflow::Tensor JetImageProducer::createImage(const edm::View<pat::Jet>& jets, const edm::Event& iEvent) const {
    // create a jet image for the leading jet in the event
    // 224 x 224 image which is centered at the jet axis and +/- 1 unit in eta and phi
    const unsigned width = 224;
    float image2D[width][width];
    float pixel_width = 2./float(width);
    for (unsigned ii = 0; ii < width; ii++){
      for (unsigned ij = 0; ij < width; ij++){ image2D[ii][ij] = 0.; }
    }
    
    int jet_ctr = 0;
    for(const auto& i_jet : jets){
      
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
        
        float dphi = i_phi - jet_phi;
        if (dphi > M_PI) dphi -= 2*M_PI;
        if (dphi < -1.*M_PI) dphi += 2*M_PI;
        float deta = i_eta - jet_eta;

        if ( deta > 1. || deta < -1. || dphi > 1. || dphi < -1.) continue; // outside of the image, shouldn't happen for AK8 jet!
        int eta_pixel_index =  (int) ((deta + 1.)/pixel_width);
        int phi_pixel_index =  (int) ((dphi + 1.)/pixel_width);
        image2D[eta_pixel_index][phi_pixel_index] += i_pt/jet_pt;

      }
  
      //////////////////////////////
      jet_ctr++;
      if (jet_ctr > 0) break; // just do one jet for now
      //////////////////////////////

    }

    // optionally output image to jpeg
    if(saveImage_){
      std::stringstream img_name;
      const auto& aux = iEvent.eventAuxiliary();
      img_name << "jet_" << jet_ctr << "_run" << aux.run() << "_lumi" << aux.luminosityBlock() << "_event" << aux.event() << ".txt";
      std::ofstream ofile(img_name.str());
      if(ofile.is_open()){
        std::stringstream img_text;
        for (unsigned itf = 0; itf < width; itf++){
          for (unsigned jtf = 0; jtf < width; jtf++){
            img_text << image2D[itf][jtf] << " ";
          }
          img_text << "\n";
        }
        ofile << img_text.str();
      }
    }

    // convert image to tensor
    tensorflow::Tensor inputImage(tensorflow::DT_FLOAT, { 1, width, width, 3 });
    auto input_map = inputImage.tensor<float, 4>();
    for (unsigned itf = 0; itf < width; itf++){
      for (unsigned jtf = 0; jtf < width; jtf++){
        input_map(0,itf,jtf,0) = image2D[itf][jtf];
        input_map(0,itf,jtf,1) = image2D[itf][jtf];
        input_map(0,itf,jtf,2) = image2D[itf][jtf];
      }
    }

    return inputImage;
}

std::vector<tensorflow::Tensor> JetImageProducer::runFeaturizer(const tensorflow::Tensor& inputImage) const {
    std::stringstream msg;
    msg << " ====> Run the Featurizer...\n";
    // Tensorflow part
    msg << "Create featurizer session...\n";
    tensorflow::Session* sessionF = tensorflow::createSession(graphDefFeaturizer_);
    msg << "Featurizer input = " << inputImage.DebugString() << "\n";
    edm::LogInfo("JetImageProducer") << msg.str();

    msg.str("");
    std::vector<tensorflow::Tensor> featurizer_outputs;
    tensorflow::Status statusF = sessionF->Run( {{"InputImage:0",inputImage}}, { "resnet_v1_50/pool5:0" }, {}, &featurizer_outputs);
    if (!statusF.ok()) { msg << statusF.ToString() << "\n"; }
    else{ msg << "Featurizer Status: Ok\n"; }
/*    msg << "featurizer_outputs vector size = " << featurizer_outputs.size() << "\n";
    msg << "featurizer_outputs vector = " << featurizer_outputs[0].DebugString() << "\n";
*/
    msg << "Close the featurizer session..."  << "\n";
    tensorflow::closeSession(sessionF);
    edm::LogInfo("JetImageProducer") << msg.str();

    return featurizer_outputs;
}

tensorflow::Tensor JetImageProducer::createFeatureList(const tensorflow::Tensor& input) const {
    tensorflow::Tensor inputClassifier(tensorflow::DT_FLOAT, { 1, 1, 1, 2048 });
    auto input_map_classifier = inputClassifier.tensor<float,4>();
    auto feature_list = input.tensor<float,4>();
    for (int itf = 0; itf < 2048; itf++){
      input_map_classifier(0,0,0,itf) = feature_list(0,0,0,itf);
    }
    return inputClassifier;
}

std::vector<tensorflow::Tensor> JetImageProducer::runClassifier(const tensorflow::Tensor& inputClassifier) const {
    std::stringstream msg;
    msg << " ====> Run the Classifier...\n";
    // Tensorflow part
    msg << "Create classifier session...\n";
    tensorflow::Session* sessionC = tensorflow::createSession(graphDefClassifier_);

    msg << "Classifier input = " << inputClassifier.DebugString() << "\n";

    msg.str("");
    std::vector<tensorflow::Tensor> outputs;
    tensorflow::Status statusC = sessionC->Run( {{"Input:0",inputClassifier}}, { "resnet_v1_50/logits/Softmax:0" }, {}, &outputs);
    if (!statusC.ok()) { msg << statusC.ToString() << "\n"; }
    else{ msg << "Classifier Status: Ok"  << "\n"; }
    // auto outputs_map_classifier = outputs[0].tensor<float,4>();
/*    msg << "output vector size = " << outputs.size() << "\n";
    msg << "output vector = " << outputs[0].DebugString() << "\n";
*/
    msg << "Close the classifier session..." << "\n";
    tensorflow::closeSession(sessionC);
    edm::LogInfo("JetImageProducer") << msg.str();

    return outputs;
}

void JetImageProducer::produce(edm::StreamID, edm::Event& iEvent, edm::EventSetup const &iSetup) const {
    edm::Handle<edm::View<pat::Jet>> h_jets;
    iEvent.getByToken(JetTok_, h_jets);

    auto t0 = std::chrono::high_resolution_clock::now();

    tensorflow::Tensor inputImage = createImage(*h_jets.product(),iEvent);

    auto t1 = std::chrono::high_resolution_clock::now();
    edm::LogInfo("JetImageProducer") << "Image time: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

	tensorflow::Tensor outputScores;

    if(remote_){
        // run the inference on the remote server and get back the result
        bool result = client_.predict(inputImage,outputScores);
        auto t2 = std::chrono::high_resolution_clock::now();
        edm::LogInfo("JetImageProducer") << "Remote prediction = " << result << ", time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }
    else {
        // --------------------------------------------------------------------
        // Run the Featurizer
        std::vector<tensorflow::Tensor> featurizer_outputs = runFeaturizer(inputImage);

        auto t2 = std::chrono::high_resolution_clock::now();
        edm::LogInfo("JetImageProducer") << "Featurizer time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        // --------------------------------------------------------------------
        // Run the Classifier
        tensorflow::Tensor inputClassifier = createFeatureList(featurizer_outputs[0]);
        std::vector<tensorflow::Tensor> outputs = runClassifier(inputClassifier);
		outputScores = outputs[0];

        auto t3 = std::chrono::high_resolution_clock::now();
        edm::LogInfo("JetImageProducer") << "Classifier time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
    }

	//check the results
	findTopN(outputScores,topN_);
}

JetImageProducer::~JetImageProducer() {
}

DEFINE_FWK_MODULE(JetImageProducer);
