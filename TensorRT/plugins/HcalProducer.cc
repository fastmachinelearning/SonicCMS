#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <cmath>
#include <utility>
#include <algorithm>

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
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

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


			this->template produces<HBHERecHitCollection>();
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
			//auto batchSize = std::distance(hRecHitHCAL->begin(), hRecHitHCAL->end());
			iInput = Input(ninput*batchSize, 0.f);
			/*for(unsigned ib = 0; ib < batchSize; ib++) { 
				for(unsigned i0 = 0; i0 < ninput; i0++) { 
					iInput[ib*ninput+0] = 1; //
					iInput[ib*ninput+1] = 1; //
					iInput[ib*ninput+2] = 1; //
					iInput[ib*ninput+3] = int(rand() % 30)-15; //
					iInput[ib*ninput+4] = int(rand() % 36)-36; //
					iInput[ib*ninput+5] = 1;
					for(unsigned i1 = 6; i1 < ninput; i1++) iInput[ib*ninput+i1] = float(rand() % 1000)*0.1;
				}
			}*/
			//batchSize == # of RHs in evt
			//auto batchSize = std::distance(hRecHitHCAL->begin(), hRecHitHCAL->end());
			std::cout << "# of RHs: " << std::distance(hRecHitHCAL->begin(), hRecHitHCAL->end()) << std::endl;

			//fill inputs
			unsigned int ib = 0;
  			for(HBHERecHitCollection::const_iterator itRH = hRecHitHCAL->begin(); itRH != hRecHitHCAL->end(); itRH++) {

				depth = (float)itRH->id().depth();
				ieta  = (float)itRH->id().ieta();
				iphi  = (float)itRH->id().iphi();

				//std::cout << sizeof(depth) << std::endl;
				iInput[ib*ninput+0] = ieta;
				iInput[ib*ninput+1] = iphi; 



  				for (HBHEChannelInfoCollection::const_iterator iter = hChannelInfo->begin(); iter != hChannelInfo->end(); iter++) {
    					const HBHEChannelInfo& pChannel(*iter);
    					const HcalDetId        pDetId = pChannel.id();
    					if(pDetId != itRH->id()) continue; 
					iInput[ib*ninput+2] = pChannel.tsGain(0);
					for (unsigned int iTS=0; iTS<8; ++iTS) {
						iInput[ib*ninput+iTS+3] = (float)pChannel.tsRawCharge(iTS);
					}
				}

				for(unsigned int d = 0; d < 8; d++){
					if(depth == (float)d) 	{ iInput[ib*ninput + d + 10] = 1.; }
					else 			{ iInput[ib*ninput + d + 10] = 0.; }
				}
				ib++;		


			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {


  			edm::Handle<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>> hRecHitHCAL_client;
  			iEvent.getByToken(fTokRH,hRecHitHCAL_client);

			std::unique_ptr<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>> out;
			out = std::make_unique<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>>();
			for(HBHERecHitCollection::const_iterator itRH = hRecHitHCAL_client->begin(); itRH != hRecHitHCAL_client->end(); itRH++){
				out->push_back(*itRH);

			}

			//check the results
			//findTopN(iOutput);
			iEvent.put(std::move(out));
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

template<> std::string HcalProducerSync::getCfiName() { return "HcalProducerSync"; }
template<> std::string HcalProducerAsync::getCfiName() { return "HcalProducerAsync"; }
template<> std::string HcalProducerPseudoAsync::getCfiName() { return "HcalProducerPseudoAsync"; }

DEFINE_FWK_MODULE(HcalProducerSync);
DEFINE_FWK_MODULE(HcalProducerAsync);
DEFINE_FWK_MODULE(HcalProducerPseudoAsync);
