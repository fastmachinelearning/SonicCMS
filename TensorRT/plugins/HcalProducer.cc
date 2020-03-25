#include <vector>
#include <map>
#include <sstream>
#include <string>

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

template <typename Client>
class HcalProducer : public SonicEDProducer<Client>
{
	public:
		//needed because base class has dependent scope
		using typename SonicEDProducer<Client>::Input;
		using typename SonicEDProducer<Client>::Output;
		explicit HcalProducer(edm::ParameterSet const& cfg) : SonicEDProducer<Client>(cfg), topN_(cfg.getParameter<unsigned>("topN")) {
			//for debugging
			this->setDebugName("HcalProducer");
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {
			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			iInput = Input(ninput*batchSize, 0.f);
			//make some random channels
			for(unsigned ib = 0; ib < batchSize; ib++) { 
				for(unsigned i0 = 0; i0 < ninput; i0++) { 
					iInput[ib*ninput+0] = 1; //
					iInput[ib*ninput+1] = 1; //
					iInput[ib*ninput+2] = 1; //
					iInput[ib*ninput+3] = int(rand() % 30)-15; //
					iInput[ib*ninput+4] = int(rand() % 36)-36; //
					iInput[ib*ninput+5] = 1;
					for(unsigned i1 = 6; i1 < ninput; i1++) iInput[ib*ninput+i1] = float(rand() % 1000)*0.1;
				}
			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
		}
		~HcalProducer() override {}

		//to ensure distinct cfi names - specialized below
		static std::string getCfiName();
		static void fillDescriptions(edm::ConfigurationDescriptions & descriptions) {
			edm::ParameterSetDescription desc;
			Client::fillPSetDescription(desc);
			desc.add<unsigned>("topN",5);
			descriptions.add(getCfiName(),desc);
		}

	private:
		using SonicEDProducer<Client>::client_;
		//Just putting something in for the hell of it
		void findTopN(const Output& scores) const {
			auto dim = client_.noutput();
			for(unsigned i0 = 0; i0 < client_.batchSize(); i0++) {
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
					if(counter>=topN_) break;
				}
				edm::LogInfo("HcalProducer") << msg.str();
			}
		}

		unsigned topN_;
};

typedef HcalProducer<TRTClientSync> HcalProducerSync;
typedef HcalProducer<TRTClientAsync> HcalProducerAsync;
typedef HcalProducer<TRTClientPseudoAsync> HcalProducerPseudoAsync;

template<> std::string HcalProducerSync::getCfiName() { return "HcalProducerSync"; }
template<> std::string HcalProducerAsync::getCfiName() { return "HcalProducerAsync"; }
template<> std::string HcalProducerPseudoAsync::getCfiName() { return "HcalProducerPseudoAsync"; }

DEFINE_FWK_MODULE(HcalProducerSync);
DEFINE_FWK_MODULE(HcalProducerAsync);
DEFINE_FWK_MODULE(HcalProducerPseudoAsync);
