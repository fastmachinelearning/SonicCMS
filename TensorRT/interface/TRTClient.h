#ifndef SonicCMS_TensorRT_TRTClient
#define SonicCMS_TensorRT_TRTClient

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "SonicCMS/Core/interface/SonicClientSync.h"
#include "SonicCMS/Core/interface/SonicClientPseudoAsync.h"
#include "SonicCMS/Core/interface/SonicClientAsync.h"

#include <vector>
#include <string>

#include "request_grpc.h"

namespace nic = nvidia::inferenceserver::client;

template <typename Client>
class TRTClient : public Client {
	public:
		//constructor
		TRTClient(const edm::ParameterSet& params);

		//helper
		void getResults(const std::unique_ptr<nic::InferContext::Result>& result);

		//accessors
		unsigned ninput() const { return ninput_; }
		unsigned noutput() const { return noutput_; }
		unsigned batchSize() const { return batchSize_; }

		//for fillDescriptions
		static void fillPSetDescription(edm::ParameterSetDescription& iDesc) {
			edm::ParameterSetDescription descClient;
			descClient.add<unsigned>("ninput");
			descClient.add<unsigned>("noutput");
			descClient.add<unsigned>("batchSize");
			descClient.add<std::string>("address");
			descClient.add<unsigned>("port");
			descClient.add<unsigned>("timeout");
			descClient.add<std::string>("modelName");
			iDesc.add<edm::ParameterSetDescription>("Client",descClient);
		}

	protected:
		void predictImpl() override;

		//helper for common ops
		void setup();

		//members
		std::string url_;
		unsigned timeout_;
		std::string modelName_;
		unsigned batchSize_;
		unsigned ninput_;
		unsigned noutput_;
		std::unique_ptr<nic::InferContext> context_;
		std::shared_ptr<nic::InferContext::Input> nicinput_; 
};
typedef TRTClient<SonicClientSync<std::vector<float>>> TRTClientSync;
typedef TRTClient<SonicClientPseudoAsync<std::vector<float>>> TRTClientPseudoAsync;
typedef TRTClient<SonicClientAsync<std::vector<float>>> TRTClientAsync;

#endif
