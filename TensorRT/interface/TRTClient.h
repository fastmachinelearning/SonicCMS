#ifndef SonicCMS_TensorRT_TRTClient
#define SonicCMS_TensorRT_TRTClient

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClientSync.h"
#include "SonicCMS/Core/interface/SonicClientPseudoAsync.h"
#include "SonicCMS/Core/interface/SonicClientAsync.h"

#include <vector>
#include <string>

#include "request_grpc.h"

namespace nic = nvidia::inferenceserver::client;

struct ServerSideStats {
  uint64_t request_count;
  uint64_t cumm_time_ns;
  uint64_t queue_time_ns;
  uint64_t compute_time_ns;

  std::map<ModelInfo, ServerSideStats> composing_models_stat;
};

using ModelInfo = std::pair<std::string, int64_t>;

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

	protected:
		void predictImpl() override;


		nic::Error ReportServerSideState(const ServerSideStats& stats);
		nic::Error SummarizeServerModelStats(
			const std::string& model_name, const int64_t model_version,
			const ni::ModelStatus& start_status, const ni::ModelStatus& end_status,
			ServerSideStats* server_stats);

		nic::Error GetServerSideStatus(std::map<std::string, ni::ModelStatus>* model_status);
		nic::Error GetServerSideStatus(
			ni::ServerStatus& server_status, const ModelInfo model_info,
			std::map<std::string, ni::ModelStatus>* model_status)

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
		std::shared_ptr<nic::ServerStatusContext> server_ctx_;
};

typedef TRTClient<SonicClientSync<std::vector<float>>> TRTClientSync;
typedef TRTClient<SonicClientPseudoAsync<std::vector<float>>> TRTClientPseudoAsync;
typedef TRTClient<SonicClientAsync<std::vector<float>>> TRTClientAsync;

#endif
