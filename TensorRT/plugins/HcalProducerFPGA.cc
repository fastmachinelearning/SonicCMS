#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/TensorRT/interface/TRTClientFPGA.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

template <typename Client>
class HcalProducerFPGA : public SonicEDProducer<Client>
{
	public:
		//needed because base class has dependent scope
		using typename SonicEDProducer<Client>::Input;
		using typename SonicEDProducer<Client>::Output;
		explicit HcalProducerFPGA(edm::ParameterSet const& cfg) : SonicEDProducer<Client>(cfg), topN_(cfg.getParameter<unsigned>("topN")) {
			//for debugging
			this->setDebugName("HcalProducerFPGA");
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {
			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			iInput = Input(ninput*batchSize, 0.f);
			//make some random channels
			for(unsigned ib = 0; ib < batchSize; ib++) {
				//Current Hcal setup takes 11 inputs but it is sent in chunks of 16 to the FPGA
				iInput[ib*ninput+0] = f_to_ui<16,6>(1);
				iInput[ib*ninput+1] = f_to_ui<16,6>(2);
				iInput[ib*ninput+3] = f_to_ui<16,6>(int(rand() % 30)-15);
				iInput[ib*ninput+4] = f_to_ui<16,6>(int(rand() % 36)-36);
				for(unsigned i0 = 5; i0 < 18; i0++) iInput[ib*ninput+i0] = f_to_ui<16,8>(1);
				for(unsigned i0 = 18; i0 < ninput; i0++) iInput[ib*ninput+i0] = f_to_ui<16,8>(0);
			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
		}
		~HcalProducerFPGA() override {}
		//to ensure distinct cfi names - specialized below
		static std::string getCfiName();
		static void fillDescriptions(edm::ConfigurationDescriptions & descriptions) {
			edm::ParameterSetDescription desc;
			Client::fillPSetDescription(desc);
			desc.add<unsigned>("topN",5);
			descriptions.add(getCfiName(),desc);
		}
		//Just putting something in for the hell of it
		void findTopN(const Output& scores) const {
			auto dim = client_.noutput();
			//int batchsize = client_.batchSize();
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
				edm::LogInfo("HcalProducerFPGA") << msg.str();
			}
		}


	private:
		using SonicEDProducer<Client>::client_;
		template<unsigned int B, unsigned int I>
		unsigned short f_to_ui(float f) {
			bool isPos = f > 0.;
			short tmpIs = int(std::abs(f));
			unsigned short tmpI = tmpIs;
			if (not isPos) {
				unsigned short comp = ((unsigned short)((1<<(sizeof(unsigned short)*4-I+1))-1)<<I);
				tmpI = -tmpIs;
				tmpI = tmpI-comp;
			}
			float tmpF = std::abs(f) - float(tmpIs);
			unsigned short fracs = tmpF*float(1<<(B-I));
			unsigned short val = (tmpI << (B-I)) + fracs;
			return val;
		}
		template<unsigned int B, unsigned int I>
		float ui_to_f(const unsigned short ui) {
			unsigned short i = ui >> (B-I);
			unsigned short mask = (1 << (B-I))-1;
			unsigned short dec = ui & mask;
			float lDec = float(dec)/float(1 << (B-I));
			return float(i)+lDec;
		}
		uint32_t merge(unsigned short iA,unsigned short iB) {
			uint32_t result = (uint32_t) iA << 16 | iB;
			return result;
		}
		unsigned topN_;
};

typedef HcalProducerFPGA<TRTClientFPGASync> HcalProducerFPGASync;
typedef HcalProducerFPGA<TRTClientFPGAAsync> HcalProducerFPGAAsync;
typedef HcalProducerFPGA<TRTClientFPGAPseudoAsync> HcalProducerFPGAPseudoAsync;

template<> std::string HcalProducerFPGASync::getCfiName() { return "HcalProducerFPGASync"; }
template<> std::string HcalProducerFPGAAsync::getCfiName() { return "HcalProducerFPGAAsync"; }
template<> std::string HcalProducerFPGAPseudoAsync::getCfiName() { return "HcalProducerFPGAPseudoAsync"; }

DEFINE_FWK_MODULE(HcalProducerFPGASync);
DEFINE_FWK_MODULE(HcalProducerFPGAAsync);
DEFINE_FWK_MODULE(HcalProducerFPGAPseudoAsync);
