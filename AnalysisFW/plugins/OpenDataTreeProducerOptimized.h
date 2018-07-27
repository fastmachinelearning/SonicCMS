#ifndef OpenDataTreeProducerOptimized_h
#define OpenDataTreeProducerOptimized_h

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "JetMETCorrections/Objects/interface/JetCorrector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/JetCorrFactors.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "fastjet/contrib/Njettiness.hh"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

using namespace edm;
using namespace reco;
using namespace std;
using namespace trigger;

class OpenDataTreeProducerOptimized : public edm::EDAnalyzer 
{
  public:

    explicit OpenDataTreeProducerOptimized(edm::ParameterSet const& cfg);
    virtual void beginJob();
    virtual void beginRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void analyze(edm::Event const& evt, edm::EventSetup const& iSetup);
    virtual void endRun(edm::Run const &, edm::EventSetup const& iSetup);
    virtual void endJob();
    virtual ~OpenDataTreeProducerOptimized();

  private:
    
    edm::InputTag JetTag_;
    edm::EDGetTokenT<edm::View<pat::Jet>> JetTok_;
    tensorflow::GraphDef* graphDef_;
    tensorflow::GraphDef* graphDefClassifier_;
    
};

#endif
