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
#include "SonicCMS/AnalysisFW/interface/TFClientRemoteTRT.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/src/PreallocationConfiguration.h"

#include "DataFormats/Math/interface/deltaR.h"


class HcalCache {
	public:
		void input(float *in) { input_=in; }
		const float* input() const { return input_; }
		float* input() { return input_; }

		const float* output() const { return output_; }
		float* output() { return output_; }
		float* output(float *out) { return output_=out; }

	private:
                float *input_;
                float *output_;
};

class HcalProducer : public edm::global::EDProducer<edm::ExternalWork,edm::StreamCache<HcalCache>>
{
	public:
		explicit HcalProducer(edm::ParameterSet const& cfg);
		void preallocate(edm::PreallocationConfiguration const& iPrealloc) override;
		std::unique_ptr<HcalCache> beginStream(edm::StreamID) const override;
		void acquire(edm::StreamID iStream, edm::Event const& iEvent, edm::EventSetup const& iSetup, edm::WaitingTaskWithArenaHolder holder) const override;
		void produce(edm::StreamID iStream, edm::Event& iEvent, edm::EventSetup const& iSetup) const;
		~HcalProducer() override;

	private:
		void findTopN(const float *scores, unsigned n=5) const;
		void createChannels(float *iImg) const;

		//edm::InputTag JetTag_;
		//edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
		unsigned topN_;
		unsigned ninput_;
		unsigned noutput_;
		unsigned batchSize_;
		bool remote_;
		edm::ParameterSet extraParams_;
		std::unique_ptr<TFClientBase> client_;
		std::string imageListFile_;
		std::vector<std::string> imageList_;
};

HcalProducer::HcalProducer(edm::ParameterSet const &cfg) :
  //JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
  //JetTok_(consumes<edm::View<pat::Jet>>(JetTag_)),
  topN_(cfg.getParameter<unsigned>("topN")),
  ninput_(cfg.getParameter<unsigned>("NIn")),
  noutput_(cfg.getParameter<unsigned>("NOut")),
  batchSize_(cfg.getParameter<unsigned>("batchSize")),
  remote_(cfg.getParameter<bool>("remote")),
  extraParams_(cfg.getParameter<edm::ParameterSet>("ExtraParams")) {
}

void HcalProducer::preallocate(edm::PreallocationConfiguration const& iPrealloc) {
  if(remote_){
    //in remote version, model is already loaded on the server
    client_ = std::make_unique<TFClientRemoteTRT>(
						  iPrealloc.numberOfStreams(),
						  extraParams_.getParameter<std::string>("address"),
						  extraParams_.getParameter<int>("port"),
						  extraParams_.getParameter<unsigned>("timeout"),
						  extraParams_.getParameter<std::string>("modelname"),
						  batchSize_,
						  ninput_,
						  noutput_
						  );
    edm::LogInfo("HcalProducer") << "Connected to remote server";
  }
}
std::unique_ptr<HcalCache> HcalProducer::beginStream(edm::StreamID) const {
  return std::make_unique<HcalCache>();
}
//Just putting something in for the hell of it
void HcalProducer::findTopN(const float *scores, unsigned n) const {
  //auto score_list = scores.flat<float>();
   auto dim = noutput_;
   for(unsigned i0 = 0; i0 < batchSize_; i0++) {
     //match score to type by index, then put in largest-first map
     std::map<float,std::string,std::greater<float>> score_map;
     for(unsigned i = 0; i < (unsigned)dim; ++i){
       std::stringstream pSS; pSS << "Dummy Channel " << i;
       score_map.emplace(scores[i0*dim+i],pSS.str());
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
     //edm::LogInfo("HcalProducer") << msg.str();
   }
}
//Make some random channel
void HcalProducer::createChannels(float *lImg) const {
  //float *lImg  = new float[ninput_*batchSize_];
  for(unsigned ib = 0; ib < batchSize_; ib++) { 
    for(unsigned i0 = 0; i0 < ninput_; i0++) { 
      lImg[ib*ninput_+0] = 1; //
      lImg[ib*ninput_+1] = 1; //
      lImg[ib*ninput_+2] = 1; //
      lImg[ib*ninput_+3] = int(rand() % 30)-15; //
      lImg[ib*ninput_+4] = int(rand() % 36)-36; //
      lImg[ib*ninput_+5] = 1;
      for(unsigned i1 = 6; i1 < ninput_; i1++) lImg[ib*ninput_+i1] = float(rand() % 1000)*0.1;
    }
  }
  //  return lImg;
}

void HcalProducer::acquire(edm::StreamID iStream, edm::Event const& iEvent, edm::EventSetup const& iSetup, edm::WaitingTaskWithArenaHolder holder) const {
	//input data from event
        //edm::Handle<edm::View<pat::Jet>> h_jets;
	//iEvent.getByToken(JetTok_, h_jets);

	//reset cache of input and output
	HcalCache* streamCacheData = streamCache(iStream);
	auto t0 = std::chrono::high_resolution_clock::now();
	float *lImg  = new float[ninput_*batchSize_];
	createChannels(lImg);
	streamCacheData->input(lImg); 
	auto t1 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("HcalProducer") << "Image time: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
	
	// run the inference on remote or local
	float *lOutput = new float[noutput_*batchSize_];
	streamCacheData->output(lOutput);
	client_->predict(iStream.value(),streamCacheData->input(),streamCacheData->output(),holder);
	auto t2 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("HcalProducer") << "Image pred: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void HcalProducer::produce(edm::StreamID iStream, edm::Event& iEvent, edm::EventSetup const &iSetup) const {
	HcalCache* streamCacheData = streamCache(iStream);
	//check the results
	findTopN(streamCacheData->output(),topN_);
}

HcalProducer::~HcalProducer() {
}

DEFINE_FWK_MODULE(HcalProducer);
