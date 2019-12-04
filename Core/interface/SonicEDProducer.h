#ifndef SonicCMS_Core_SonicEDProducer
#define SonicCMS_Core_SonicEDProducer

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//this is a stream producer because client operations are not multithread-safe in general
//it is designed such that the user never has to interact with the client or the acquire() callback directly
template <typename Client, typename... Capabilities>
class SonicEDProducer : public edm::stream::EDProducer<edm::ExternalWork, Capabilities...> {
	public:
		//constructor
		SonicEDProducer(edm::ParameterSet const& cfg) : client_(cfg.getParameter<edm::ParameterSet>("Client")) {}
		//destructor
		virtual ~SonicEDProducer() {}
		
		//derived classes just implement load, not acquire
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, edm::WaitingTaskWithArenaHolder holder) override final {
			client_.setInput(load(iEvent, iSetup));
			client_.predict(holder);
		}
		virtual typename Client::Input load(edm::Event const& iEvent, edm::EventSetup const& iSetup) = 0;
		//derived classes use a dedicated produce() interface that incorporates client_.output()
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup) override final {
			//todo: measure time between acquire and produce
			produce(iEvent, iSetup, client_.output());
		}
		virtual void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, typename Client::Output const& iOutput) = 0;
		
	protected:
		//members
		Client client_;
};

#endif

