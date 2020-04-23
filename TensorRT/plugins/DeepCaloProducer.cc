#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

template <typename Client>
class DeepCaloProducer : public SonicEDProducer<Client>
{
	public:
		//needed because base class has dependent scope
		using typename SonicEDProducer<Client>::Input;
		using typename SonicEDProducer<Client>::Output;
		explicit DeepCaloProducer(edm::ParameterSet const& cfg) : 
			SonicEDProducer<Client>(cfg), 
			topN_(cfg.getParameter<unsigned>("topN")),
			binDataPath_(cfg.getParameter<std::string>("binDataPath")) 
		{
			//for debugging
			this->setDebugName("DeepCaloProducer");

			// Load the bin data
			std::streampos fileSize;
			std::ifstream file(binDataPath_, std::ios::binary);

			if(file.is_open()){
				file.unsetf(std::ios::skipws);

				// Get the size of the file
				file.seekg(0, std::ios::end);
				fileSize = file.tellg();
				file.seekg(0, std::ios::beg);

				imageData_.reserve(fileSize/sizeof(float));

				for (size_t i = 0; i < imageData_.capacity(); i++)
				{
					float f;
					file.read(reinterpret_cast<char*>(&f), sizeof(float));
					imageData_.push_back(f);
				}
				file.close();
			} else throw cms::Exception("MissingInputFile") << "Could not read the file: " << binDataPath_;

			imageID_ = 0;
			imageN_ = imageData_.size() / (56*11*4);
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {
			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			iInput = Input(ninput*batchSize, 0.f);

			if (imageID_ + batchSize >= imageN_)
				imageID_ = 0;

			unsigned int start = imageID_*56*11*4;
			unsigned int end   = start + batchSize*56*11*4;

			iInput = std::vector<float>(imageData_.begin() + start, imageData_.begin() + end);

			imageID_ += batchSize;
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			findTopN(iOutput);
		}
		~DeepCaloProducer() override {}

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
				edm::LogInfo("DeepCaloProducer") << msg.str();
			}
		}

		unsigned topN_;
		unsigned int imageID_;
		unsigned int imageN_;
		std::string binDataPath_;
		std::vector<float> imageData_;
};

typedef DeepCaloProducer<TRTClientSync> DeepCaloProducerSync;
typedef DeepCaloProducer<TRTClientAsync> DeepCaloProducerAsync;
typedef DeepCaloProducer<TRTClientPseudoAsync> DeepCaloProducerPseudoAsync;

DEFINE_FWK_MODULE(DeepCaloProducerSync);
DEFINE_FWK_MODULE(DeepCaloProducerAsync);
DEFINE_FWK_MODULE(DeepCaloProducerPseudoAsync);
