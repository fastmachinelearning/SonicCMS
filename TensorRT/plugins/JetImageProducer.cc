#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <map>

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

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

template <typename Client>
class JetImageProducer : public SonicEDProducer<Client>
{
	public:
		//needed because base class has dependent scope
		using typename SonicEDProducer<Client>::Input;
		using typename SonicEDProducer<Client>::Output;
		explicit JetImageProducer(edm::ParameterSet const& cfg) :
			SonicEDProducer<Client>(cfg),
			JetTag_(cfg.getParameter<edm::InputTag>("JetTag")),
			JetTok_(this->template consumes<edm::View<pat::Jet>>(JetTag_)),
			topN_(cfg.getParameter<unsigned>("topN")),
			imageListFile_(cfg.getParameter<std::string>("imageList"))
		{
			//for debugging
			this->setDebugName("JetImageProducer");
			//load score list
			std::ifstream ifile(imageListFile_);
			if(ifile.is_open()){
				std::string line;
				while(std::getline(ifile,line)){
					imageList_.push_back(line);
				}
			}
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {
			//input data from event
			edm::Handle<edm::View<pat::Jet>> h_jets;
			iEvent.getByToken(JetTok_, h_jets);
			const auto& jets = *h_jets.product();

			// create a jet image for the leading jet in the event
			// 224 x 224 image which is centered at the jet axis and +/- 1 unit in eta and phi
			std::vector<float> img(client_.ninput(),0.f);
			const unsigned npix = 224;
			float pixel_width = 2./float(npix);

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
					img[3*(eta_pixel_index*npix+phi_pixel_index)+0] += i_pt/jet_pt;
					img[3*(eta_pixel_index*npix+phi_pixel_index)+1] += i_pt/jet_pt;
					img[3*(eta_pixel_index*npix+phi_pixel_index)+2] += i_pt/jet_pt;
				}

				//////////////////////////////
				jet_ctr++;
				if (jet_ctr > 0) break; // just do one jet for now
				//////////////////////////////
			}

			iInput = Input(client_.ninput()*client_.batchSize(),0.f);
			for(unsigned i0 = 0; i0 < client_.batchSize(); i0++ ) { 
				for(unsigned i1 = 0; i1 < client_.ninput(); i1++) {
					iInput[client_.ninput()*i0+i1] = img[i1];
				}
			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
		}
		~JetImageProducer() override {}

		//to ensure distinct cfi names - specialized below
		static std::string getCfiName();
		static void fillDescriptions(edm::ConfigurationDescriptions & descriptions) {
			edm::ParameterSetDescription desc;
			Client::fillPSetDescription(desc);
			desc.add<edm::InputTag>("JetTag",edm::InputTag("slimmedJetsAK8"));
			desc.add<unsigned>("topN",5);
			desc.add<std::string>("imageList");
			descriptions.add(getCfiName(),desc);
		}

	private:
		using SonicEDProducer<Client>::client_;
		void findTopN(const std::vector<float>& scores, unsigned n=5) const {
			auto dim = client_.noutput();
			for(unsigned i0 = 0; i0 < client_.batchSize(); i0++) {
				//match score to type by index, then put in largest-first map
				std::map<float,std::string,std::greater<float>> score_map;
				for(unsigned i = 0; i < std::min((unsigned)dim,(unsigned)imageList_.size()); ++i){
					score_map.emplace(scores[i0*dim+i],imageList_[i]);
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
				edm::LogInfo("JetImageProducer") << msg.str();
			}
		}
		std::vector<float> createImage(const edm::View<pat::Jet>& jets) const;

		edm::InputTag JetTag_;
		edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
		unsigned topN_;
		std::string imageListFile_;
		std::vector<std::string> imageList_;
};

typedef JetImageProducer<TRTClientSync> JetImageProducerSync;
typedef JetImageProducer<TRTClientAsync> JetImageProducerAsync;
typedef JetImageProducer<TRTClientPseudoAsync> JetImageProducerPseudoAsync;

template<> std::string JetImageProducerSync::getCfiName() { return "JetImageProducerSync"; }
template<> std::string JetImageProducerAsync::getCfiName() { return "JetImageProducerAsync"; }
template<> std::string JetImageProducerPseudoAsync::getCfiName() { return "JetImageProducerPseudoAsync"; }

DEFINE_FWK_MODULE(JetImageProducerSync);
DEFINE_FWK_MODULE(JetImageProducerAsync);
DEFINE_FWK_MODULE(JetImageProducerPseudoAsync);

