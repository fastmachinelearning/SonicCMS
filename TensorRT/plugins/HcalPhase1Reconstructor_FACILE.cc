#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <cmath>
#include <utility>
#include <algorithm>
#include <fstream>
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "Geometry/HcalCommonData/interface/HcalHitRelabeller.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitDefs.h"
#include "EventFilter/HcalRawToDigi/interface/HcalPacker.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/HcalDetId/interface/HcalGenericDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/METReco/interface/HcalPhase1FlagLabels.h"

#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"


#include "CalibFormats/CaloObjects/interface/CaloSamples.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalSiPMnonlinearity.h"
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
#include "FWCore/Framework/interface/stream/EDProducer.h"
namespace {
    template<class DFrame>
    class RawChargeFromSample
    {
    public:
        inline RawChargeFromSample(const int sipmQTSShift,
                                   const int sipmQNTStoSum,
                                   const HcalDbService& cond,
                                   const HcalDetId id,
                                   const CaloSamples& cs,
                                   const int soi,
                                   const DFrame& frame,
                                   const int maxTS) {}

        inline double getRawCharge(const double decodedCharge,
                                   const double pedestal) const
            {return decodedCharge;}
    };
    template<>
    class RawChargeFromSample<QIE11DataFrame>
    {
    public:
        inline RawChargeFromSample(const int sipmQTSShift,
                                   const int sipmQNTStoSum,
                                   const HcalDbService& cond,
                                   const HcalDetId id,
                                   const CaloSamples& cs,
                                   const int soi,
                                   const QIE11DataFrame& frame,
                                   const int maxTS)
            : siPMParameter_(*cond.getHcalSiPMParameter(id)),
              fcByPE_(siPMParameter_.getFCByPE()),
              corr_(cond.getHcalSiPMCharacteristics()->getNonLinearities(siPMParameter_.getType()))
        {
            if (fcByPE_ <= 0.0)
                throw cms::Exception("HBHEPhase1BadDB")
                    << "Invalid fC/PE conversion factor for SiPM " << id
                    << std::endl;

            const HcalCalibrations& calib = cond.getHcalCalibrations(id);
            const int firstTS = std::max(soi + sipmQTSShift, 0);
            const int lastTS = std::min(firstTS + sipmQNTStoSum, maxTS);
            double sipmQ = 0.0;

            for (int ts = firstTS; ts < lastTS; ++ts)
            {
                const double pedestal = calib.pedestal(frame[ts].capid());
                sipmQ += (cs[ts] - pedestal);
            }

            const double effectivePixelsFired = sipmQ/fcByPE_;
            factor_ = corr_.getRecoCorrectionFactor(effectivePixelsFired);
        }

        inline double getRawCharge(const double decodedCharge,
                                   const double pedestal) const
        {
            return (decodedCharge - pedestal)*factor_ + pedestal;

            // Old version of TS-by-TS corrections looked as follows:
            // const double sipmQ = decodedCharge - pedestal;
            // const double nPixelsFired = sipmQ/fcByPE_;
            // return sipmQ*corr_.getRecoCorrectionFactor(nPixelsFired) + pedestal;
       }

    private:
        const HcalSiPMParameter& siPMParameter_;
        double fcByPE_;
        HcalSiPMnonlinearity corr_;
        double factor_;
    };

}

template <typename Client>
class HcalPhase1Reconstructor_FACILE : public SonicEDProducer<Client>
{
	public:
		//needed because base class has dependent scope
		using typename SonicEDProducer<Client>::Input;
		using typename SonicEDProducer<Client>::Output;
		explicit HcalPhase1Reconstructor_FACILE(edm::ParameterSet const& cfg) : 
			SonicEDProducer<Client>(cfg), 
			sipmQTSShift_(cfg.getParameter<unsigned>("sipmQTSShift")),
			sipmQNTStoSum_(cfg.getParameter<unsigned>("sipmQNTStoSum")), 
			topN_(cfg.getParameter<unsigned>("topN")),  
			fDigiName(cfg.getParameter<edm::InputTag>("digiLabelQIE11")),
			fRHName(cfg.getParameter<edm::InputTag>("edmRecHitName")),   
			fChanInfoName(cfg.getParameter<edm::InputTag>("edmChanInfoName")), 
			fTokRH(this->template consumes<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>> >(fRHName)), 
			fTokChanInfo(this->template consumes<edm::SortedCollection<HBHEChannelInfo,edm::StrictWeakOrdering<HBHEChannelInfo>> >(fChanInfoName)),
			fTokDigis(this->template consumes<QIE11DigiCollection>(fDigiName))
		{


			this->template produces<HBHERecHitCollection>();
			this->setDebugName("HcalPhase1Reconstructor_FACILE");
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {

			auto ninput = client_.ninput();
			auto batchSize = client_.batchSize();
			iInput = Input(ninput*batchSize, 0.f);

			edm::Handle<QIE11DigiCollection> digis;
			iEvent.getByToken(fTokDigis, digis);

			edm::ESHandle<HcalDbService> conditions;
			iSetup.get<HcalDbRecord>().get(conditions);

			tmp->clear();

		        processData<QIE11DataFrame>(*digis, *conditions, iInput, ninput);
			
		}

		template<class DFrame, class Collection>
		void processData(const Collection& coll,
                                 const HcalDbService& cond,
				 Input& iInput,
				 auto ninput)
		{

			const bool skipDroppedChannels = false;

			unsigned int ib = 0;
			for (typename Collection::const_iterator it = coll.begin(); it != coll.end(); it++){



				unsigned int ib = std::distance(coll.begin(),it);

			 	const DFrame& frame(*it);
	        	  	const HcalDetId cell(frame.id());

        		   	const HcalSubdetector subdet = cell.subdet();
        			if (!(subdet == HcalSubdetector::HcalBarrel ||
	   			      subdet == HcalSubdetector::HcalEndcap ||
    			              subdet == HcalSubdetector::HcalOuter))
        		    	continue;
		
				const HcalCalibrations& calib = cond.getHcalCalibrations(cell);
			        const HcalQIECoder* channelCoder = cond.getHcalCoder(cell);
			        const HcalQIEShape* shape = cond.getHcalShape(channelCoder);
			        const HcalCoderDb coder(*channelCoder, *shape);

				CaloSamples cs;
        			coder.adc2fC(frame, cs);


				const int nRead = cs.size();
			        const int maxTS = std::min(nRead, static_cast<int>(HBHEChannelInfo::MAXSAMPLES));

				const int soi = 3;
				const int nCycles = 8;
			        const RawChargeFromSample<DFrame> rcfs(sipmQTSShift_, sipmQNTStoSum_, 
                                               			       cond, cell, cs, soi, frame, maxTS);


				iInput[ib*ninput + 0] = (float)cell.iphi();
				for (int inputTS = 0; inputTS < nCycles; ++inputTS){
					auto s(frame[inputTS]);
					const uint8_t adc = s.adc();
					const int capid = s.capid();
			        	const double gain = calib.respcorrgain(capid);	
					iInput[ib*ninput + 1] = (float)gain;
				        const double rawCharge = rcfs.getRawCharge(cs[inputTS], calib.pedestal(capid));
					iInput[ib*ninput+inputTS+2] = ((float)rawCharge);
				}
				for(unsigned int d = 1; d < 8; d++){
					if(cell.depth() == (float)d) 	{ iInput[ib*ninput + d + 9] = 1.; }
					else 				{ iInput[ib*ninput + d + 9] = 0.; }
				}
				for(unsigned int d = 0; d < 30; d++){
					if(std::abs(cell.ieta()) == (float)d) 	{ iInput[ib*ninput + d + 17] = 1.; }
					else 					{ iInput[ib*ninput + d + 17] = 0.; }
				}
				ib++;
				HBHERecHit rh = HBHERecHit(cell, 0.f,0.f,0.f);
				tmp->push_back(rh);
			}
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {


  			std::unique_ptr<HBHERecHitCollection> out;
			out = std::make_unique<HBHERecHitCollection>();

			unsigned int ib = 0;
			for(HBHERecHitCollection::const_iterator it = tmp->begin(); it != tmp->end(); it++){

				float rh_e = iOutput[ib];
				if (rh_e < 0.01) rh_e = 0.01;
				else if (rh_e > 1000.) rh_e = 1000.;
				HBHERecHit rhout = HBHERecHit(it->id(),rh_e,0.f,0.f);
				out->push_back(rhout); ib++;
			}
			iEvent.put(std::move(out));
		}
		~HcalPhase1Reconstructor_FACILE() override {}

	private:



		int sipmQTSShift_;
		int sipmQNTStoSum_;
		unsigned topN_;
	  	edm::InputTag fDigiName;
    		edm::InputTag fRHName;
    		edm::InputTag fChanInfoName;
   		edm::EDGetTokenT<edm::SortedCollection<HBHERecHit,edm::StrictWeakOrdering<HBHERecHit>>> fTokRH;
    		edm::EDGetTokenT<edm::SortedCollection<HBHEChannelInfo,edm::StrictWeakOrdering<HBHEChannelInfo>>> fTokChanInfo;
 		edm::EDGetTokenT<QIE11DigiCollection> fTokDigis;

                std::vector<HBHERecHit> tmprh;
		std::vector<HBHERecHit> *tmp = &tmprh;
		
		float depth, ieta, iphi; 

		using SonicEDProducer<Client>::client_;
                template<unsigned int B, unsigned int I>
                unsigned short f_to_ui(float f) {
                    bool isPos = f > 0.;
                    short tmpIs = int(fabs(f));
                    unsigned short tmpI = tmpIs;
                    if (not isPos) {
		      unsigned short comp = ((unsigned short)((1<<(sizeof(unsigned short)*4-I+1))-1)<<I);
		      tmpI = -tmpIs;
		      tmpI = tmpI-comp;
                    }
                    float tmpF = fabs(f) - float(tmpIs);
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
};


typedef HcalPhase1Reconstructor_FACILE<TRTClientSync> HcalPhase1Reconstructor_FACILESync;
typedef HcalPhase1Reconstructor_FACILE<TRTClientAsync> HcalPhase1Reconstructor_FACILEAsync;
typedef HcalPhase1Reconstructor_FACILE<TRTClientPseudoAsync> HcalPhase1Reconstructor_FACILEPseudoAsync;

DEFINE_FWK_MODULE(HcalPhase1Reconstructor_FACILESync);
DEFINE_FWK_MODULE(HcalPhase1Reconstructor_FACILEAsync);
DEFINE_FWK_MODULE(HcalPhase1Reconstructor_FACILEPseudoAsync);
