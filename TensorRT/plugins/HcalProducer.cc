#include <vector>
#include <map>
#include <sstream>
#include <string>

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

template <typename Client>
class HcalProducer : public SonicEDProducer<Client>
{
	public:
		explicit HcalProducer(edm::ParameterSet const& cfg) : SonicEDProducer(cfg), topN_(cfg.getParameter<unsigned>("topN") {}
		Client::Input load(edm::Event const& iEvent, edm::EventSetup const& iSetup) override {
			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			Client::Input lImg(ninput*batchSize, 0.f);
			//make some random channels
			for(unsigned ib = 0; ib < batchSize; ib++) { 
				for(unsigned i0 = 0; i0 < ninput; i0++) { 
					lImg[ib*ninput+0] = 1; //
					lImg[ib*ninput+1] = 1; //
					lImg[ib*ninput+2] = 1; //
					lImg[ib*ninput+3] = int(rand() % 30)-15; //
					lImg[ib*ninput+4] = int(rand() % 36)-36; //
					lImg[ib*ninput+5] = 1;
					for(unsigned i1 = 6; i1 < ninput; i1++) lImg[ib*ninput+i1] = float(rand() % 1000)*0.1;
				}
			}
			return lImg;
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Client::Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
		}
		~HcalProducer() override {}

	private:
		//Just putting something in for the hell of it
		void findTopN(const Client::Output& scores) const {
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

typedef HcalProducerSync HcalProducer<TRTClientSync>;
typedef HcalProducerAsync HcalProducer<TRTClientAsync>;
typedef HcalProducerPseudoAsync HcalProducer<TRTClientPseudoAsync>;

DEFINE_FWK_MODULE(HcalProducerSync);
DEFINE_FWK_MODULE(HcalProducerAsync);
DEFINE_FWK_MODULE(HcalProducerPseudoAsync);
