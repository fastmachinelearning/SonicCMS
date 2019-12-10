#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <map>

#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "SonicCMS/Brainwave/interface/TFClientRemote.h"
#include "SonicCMS/Brainwave/interface/TFClientLocal.h"
#include "FWCore/Framework/interface/Event.h"
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
			// create a jet image for the leading jet in the event
			// 224 x 224 image which is centered at the jet axis and +/- 1 unit in eta and phi
			const unsigned npix = 224;
			float image2D[npix][npix];
			float pixel_width = 2./float(npix);
			for (int ii = 0; ii < npix; ii++){
				for (int ij = 0; ij < npix; ij++){ image2D[ii][ij] = 0.; }
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
			iInput = Input(tensorflow::DT_FLOAT, { 1, npix, npix, 3 });
			auto input_map = inputImage.tensor<float, 4>();
			for (int itf = 0; itf < npix; itf++){
				for (int jtf = 0; jtf < npix; jtf++){
					input_map(0,itf,jtf,0) = image2D[itf][jtf];
					input_map(0,itf,jtf,1) = image2D[itf][jtf];
					input_map(0,itf,jtf,2) = image2D[itf][jtf];
				}
			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//check the results
			findTopN(iOutput);
		}

	private:
		void findTopN(const tensorflow::Tensor& scores) const {
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
				if(counter>=topN_) break;
			}
			edm::LogInfo("JetImageProducer") << msg.str();
		}

		edm::InputTag JetTag_;
		edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
		unsigned topN_;
		std::string imageListFile_;
		std::vector<std::string> imageList_;
};

typedef JetImageProducer<TFClientRemote> JetImageProducerRemote;
typedef JetImageProducer<TFClientLocal> JetImageProducerLocal;

DEFINE_FWK_MODULE(JetImageProducerRemote);
DEFINE_FWK_MODULE(JetImageProducerLocal);
