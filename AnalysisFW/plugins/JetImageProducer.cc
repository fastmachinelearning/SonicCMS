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

#include "SonicCMS/AnalysisFW/interface/TFClientBase.h"
#include "SonicCMS/AnalysisFW/interface/TFClientRemote.h"
#include "SonicCMS/AnalysisFW/interface/TFClientLocal.h"
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

#include "tensorflow/core/framework/tensor.h"

class JetImageProducer : public edm::global::EDProducer<>
{
	public:
		explicit JetImageProducer(edm::ParameterSet const& cfg);
		void produce(edm::StreamID, edm::Event& iEvent, edm::EventSetup const& iSetup) const;
		~JetImageProducer() override;

	private:
		void findTopN(const tensorflow::Tensor& scores, unsigned n=5) const;
		tensorflow::Tensor createImage(const edm::View<pat::Jet>& jets) const;

		edm::InputTag JetTag_;
		edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
		std::unique_ptr<TFClientBase> client_;
		unsigned topN_;
		std::vector<std::string> imageList_;
};

JetImageProducer::JetImageProducer(edm::ParameterSet const &cfg) :
	JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
	JetTok_(consumes<edm::View<pat::Jet>>(JetTag_)),
	topN_(cfg.getParameter<unsigned>("topN"))
{
	if(cfg.exists("ServerParams")){
		//in remote version, model is already loaded on the server
		const auto& scfg = cfg.getParameter<edm::ParameterSet>("ServerParams");
		client_ = std::make_unique<TFClientRemote>(scfg.getParameter<std::string>("address"),scfg.getParameter<int>("port"),scfg.getParameter<unsigned>("timeout"));
		edm::LogInfo("JetImageProducer") << "Connected to remote server";
	}
	else {
		client_ = std::make_unique<TFClientLocal>("resnet50.pb","resnet50_classifier.pb");
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

tensorflow::Tensor JetImageProducer::createImage(const edm::View<pat::Jet>& jets) const {
	// create a jet image for the leading jet in the event
	// 224 x 224 image which is centered at the jet axis and +/- 1 unit in eta and phi
	float image2D[224][224];
	float pixel_width = 2./224.;
	for (int ii = 0; ii < 224; ii++){
		for (int ij = 0; ij < 224; ij++){ image2D[ii][ij] = 0.; }
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

	// convert image to tensor
	tensorflow::Tensor inputImage(tensorflow::DT_FLOAT, { 1, 224, 224, 3 });
	auto input_map = inputImage.tensor<float, 4>();
	for (int itf = 0; itf < 224; itf++){
		for (int jtf = 0; jtf < 224; jtf++){
			input_map(0,itf,jtf,0) = image2D[itf][jtf];
			input_map(0,itf,jtf,1) = image2D[itf][jtf];
			input_map(0,itf,jtf,2) = image2D[itf][jtf];
		}
	}

	return inputImage;
}

void JetImageProducer::produce(edm::StreamID, edm::Event& iEvent, edm::EventSetup const &iSetup) const {
	edm::Handle<edm::View<pat::Jet>> h_jets;
	iEvent.getByToken(JetTok_, h_jets);

	auto t0 = std::chrono::high_resolution_clock::now();

	tensorflow::Tensor inputImage = createImage(*h_jets.product());

	auto t1 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("JetImageProducer") << "Image time: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

	// run the inference on remote or local
	tensorflow::Tensor outputScores;
	client_->predict(inputImage,outputScores);

	//check the results
	findTopN(outputScores,topN_);
}

JetImageProducer::~JetImageProducer() {
}

DEFINE_FWK_MODULE(JetImageProducer);
