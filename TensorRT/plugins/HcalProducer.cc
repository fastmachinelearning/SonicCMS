#include <vector>
#include <map>
#include <sstream>
#include <string>

#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "Geometry/HcalCommonData/interface/HcalHitRelabeller.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitDefs.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
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
		explicit HcalProducer(edm::ParameterSet const& cfg) : 
			SonicEDProducer<Client>(cfg), 
			topN_(cfg.getParameter<unsigned>("topN")),   
			fRHName(cfg.getParameter<edm::InputTag>("edmRecHitName")),   
			fChanInfoName(cfg.getParameter<edm::InputTag>("edmChanInfoName")), 
			fTokRH(this->template consumes<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>> >(fRHName)), 
			fTokChanInfo(this->template consumes<edm::SortedCollection<HBHEChannelInfo,edm::StrictWeakOrdering<HBHEChannelInfo>> >(fChanInfoName))
		{



			//for debugging
			this->setDebugName("HcalProducer");
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {


			//load RecHit and ChannelInfo collections from hbheprereco
  			edm::Handle<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>> hRecHitHCAL;
  			iEvent.getByToken(fTokRH,hRecHitHCAL);
  			edm::Handle<edm::SortedCollection<HBHEChannelInfo,edm::StrictWeakOrdering<HBHEChannelInfo>>> hChannelInfo;
  			iEvent.getByToken(fTokChanInfo, hChannelInfo);

			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			iInput = Input(ninput*batchSize, 0.f);

			//batchSize == # of RHs in evt
			auto batchSize = std::distance(hRecHitHCAL->begin(), hRecHitHCAL->end());

			//fill inputs
			unsigned int ib = 0;
  			for(HBHERecHitCollection::const_iterator itRH = hRecHitHCAL->begin(); itRH != hRecHitHCAL->end(); itRH++) {

				depth = (float)itRH->id().depth();
				ieta  = (float)itRH->id().ieta();
				iphi  = (float)itRH->id().iphi();

				iInput[ib*ninput+0] = ieta;
				iInput[ib*ninput+1] = iphi; 
				if(depth == 1.) iInput[ib*ninput+2] = 1.; else iInput[ib*ninput+2] = 0.;
				if(depth == 2.) iInput[ib*ninput+3] = 1.; else iInput[ib*ninput+3] = 0.;
				if(depth == 3.) iInput[ib*ninput+4] = 1.; else iInput[ib*ninput+4] = 0.;
				if(depth == 4.) iInput[ib*ninput+5] = 1.; else iInput[ib*ninput+5] = 0.;

				std::cout << "ieta/iphi/depth \t" << ieta << "/" << iphi << "/" << depth << std::endl;

  				for (HBHEChannelInfoCollection::const_iterator iter = hChannelInfo->begin(); iter != hChannelInfo->end(); iter++) {
    					const HBHEChannelInfo& pChannel(*iter);
    					const HcalDetId        pDetId = pChannel.id();
    					if(pDetId != itRH->id()) continue; 
					iInput[ib*ninput+6] = pChannel.tsGain(0);
					for (unsigned int iTS=0; iTS<8; ++iTS) {
						iInput[ib*ninput+iTS+7] = pChannel.tsRawCharge(iTS);
						//std::cout << pChannel.tsRawCharge << "/";
					}
				}
				ib++;		

			}

		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
			for(unsigned int it = 0 ; it < sizeof(iOutput)/sizeof(iOutput[0]); it++){
				std::cout << iOutput[it] << "/";
			}

		}
		~HcalProducer() override {}

	private:

//		edm::EDGetTokenT<QIE11DigiCollection> tok_qie11_;

		unsigned topN_;
    		edm::InputTag fRHName;
    		edm::InputTag fChanInfoName;
   		edm::EDGetTokenT<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>> fTokRH;
    		edm::EDGetTokenT<edm::SortedCollection<HBHEChannelInfo,edm::StrictWeakOrdering<HBHEChannelInfo>>> fTokChanInfo;

		float depth, ieta, iphi; 

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

};

typedef HcalProducer<TRTClientSync> HcalProducerSync;
typedef HcalProducer<TRTClientAsync> HcalProducerAsync;
typedef HcalProducer<TRTClientPseudoAsync> HcalProducerPseudoAsync;

DEFINE_FWK_MODULE(HcalProducerSync);
DEFINE_FWK_MODULE(HcalProducerAsync);
DEFINE_FWK_MODULE(HcalProducerPseudoAsync);
