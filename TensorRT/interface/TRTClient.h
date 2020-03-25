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
namespace ni = nvidia::inferenceserver;

using ModelInfo = std::pair<std::string, int64_t>;

struct ServerSideStats {
  uint64_t request_count;
  uint64_t cumm_time_ns;
  uint64_t queue_time_ns;
  uint64_t compute_time_ns;

  std::map<ModelInfo, ServerSideStats> composing_models_stat;
};

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

		void ReportServerSideState(const ServerSideStats& stats);
		void SummarizeServerStats(
			const ModelInfo model_info,
			const std::map<std::string, ni::ModelStatus>& start_status,
			const std::map<std::string, ni::ModelStatus>& end_status,
			ServerSideStats* server_stats);
		void SummarizeServerModelStats(
			const std::string& model_name, const int64_t model_version,
			const ni::ModelStatus& start_status, const ni::ModelStatus& end_status,
			ServerSideStats* server_stats);

		void GetServerSideStatus(std::map<std::string, ni::ModelStatus>* model_status);
		void GetServerSideStatus(
			ni::ServerStatus& server_status, const ModelInfo model_info,
			std::map<std::string, ni::ModelStatus>* model_status);

		//members
		std::string url_;
		unsigned timeout_;
		std::string modelName_;
		unsigned batchSize_;
		unsigned ninput_;
		unsigned noutput_;
		std::unique_ptr<nic::InferContext> context_;
		std::unique_ptr<nic::ServerStatusContext> server_ctx_;
		std::shared_ptr<nic::InferContext::Input> nicinput_; 

		std::map<std::string, ni::ModelStatus> start_status, end_status;
};

typedef TRTClient<SonicClientSync<std::vector<float>>> TRTClientSync;
typedef TRTClient<SonicClientPseudoAsync<std::vector<float>>> TRTClientPseudoAsync;
typedef TRTClient<SonicClientAsync<std::vector<float>>> TRTClientAsync;

#endif
