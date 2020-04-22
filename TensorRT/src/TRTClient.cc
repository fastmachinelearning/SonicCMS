#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"

#include "request_grpc.h"

#include <string>
#include <chrono>
#include <exception>

namespace nic = nvidia::inferenceserver::client;
namespace ni = nvidia::inferenceserver;

using ModelInfo = std::pair<std::string, int64_t>;

//based on https://github.com/NVIDIA/tensorrt-inference-server/blob/master/src/clients/c++/examples/simple_callback_client.cc

template <typename Client>
TRTClient<Client>::TRTClient(const edm::ParameterSet &params) : Client(),
																url_(params.getParameter<std::string>("address") + ":" + std::to_string(params.getParameter<unsigned>("port"))),
																timeout_(params.getParameter<unsigned>("timeout")),
																modelName_(params.getParameter<std::string>("modelName")),
																batchSize_(params.getParameter<unsigned>("batchSize")),
																ninput_(params.getParameter<unsigned>("ninput")),
																noutput_(params.getParameter<unsigned>("noutput"))
{
}

template <typename Client>
void TRTClient<Client>::setup()
{
	auto err = nic::InferGrpcContext::Create(&context_, url_, modelName_, -1, false);
	if (!err.IsOk())
		throw cms::Exception("BadGrpc") << "unable to create inference context: " << err;

	// nic::ServerStatusGrpcContext::Create(&server_ctx_, url_, false);
	// if (!err.IsOk())
	// 	throw cms::Exception("BadServer") << "unable to create server inference context: " << err;

	std::unique_ptr<nic::InferContext::Options> options;
	nic::InferContext::Options::Create(&options);

	options->SetBatchSize(batchSize_);
	for (const auto &output : context_->Outputs())
	{
		options->AddRawResult(output);
	}
	context_->SetRunOptions(*options);

	const std::vector<std::shared_ptr<nic::InferContext::Input>> &nicinputs = context_->Inputs();
	nicinput_ = nicinputs[0];
	nicinput_->Reset();

	auto t2 = std::chrono::high_resolution_clock::now();
	std::vector<int64_t> input_shape;
	for (unsigned i0 = 0; i0 < batchSize_; i0++)
	{
		float *arr = &(this->input_.data()[i0 * ninput_]);
		nic::Error err1 = nicinput_->SetRaw(reinterpret_cast<const uint8_t *>(arr), ninput_ * sizeof(float));
	}
	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TRTClient") << "Image array time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
}

template <typename Client>
void TRTClient<Client>::getResults(const std::unique_ptr<nic::InferContext::Result> &result)
{
	auto t2 = std::chrono::high_resolution_clock::now();
	this->output_.resize(noutput_ * batchSize_, 0.f);
	for (unsigned i0 = 0; i0 < batchSize_; i0++)
	{
		const uint8_t *r0;
		size_t content_byte_size;
		result->GetRaw(i0, &r0, &content_byte_size);
		const float *lVal = reinterpret_cast<const float *>(r0);
		for (unsigned i1 = 0; i1 < noutput_; i1++)
			this->output_[i0 * noutput_ + i1] = lVal[i1]; //This should be replaced with a memcpy
	}
	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TRTClient") << "Output time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
}

template <typename Client>
void TRTClient<Client>::predictImpl()
{
	//common operations first
	setup();

	//blocking call
	auto t2 = std::chrono::high_resolution_clock::now();
	std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
	nic::Error err = context_->Run(&results);
	if (!err.IsOk()) {
		std::cout << "Could not read the result" <<  ": " << err << std::endl;
		this->output_.resize(noutput_ * batchSize_, 0.f);
	} else {
	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TRTClient") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
		getResults(results.begin()->second);
	}
}

//specialization for true async
template <>
void TRTClientAsync::predictImpl()
{
	//common operations first
	try
	{
		setup();
	}
	catch (...)
	{
		finish(std::current_exception());
		return;
	}

	//non-blocking call

	// Get the status of the server prior to the request being made.
	// std::map<std::string, ni::ModelStatus> start_status;
	// GetServerSideStatus(&start_status);

	auto t2 = std::chrono::high_resolution_clock::now();
	nic::Error erro0 = context_->AsyncRun(
		[t2, this](nic::InferContext *ctx, const std::shared_ptr<nic::InferContext::Request> &request) {
			//get results
			std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
			//this function interface will change in the next tensorrtis version
			bool is_ready = false;
			nic::Error err = ctx->GetAsyncRunResults(&results, &is_ready, request, false);
			if (is_ready == false)
				finish(std::make_exception_ptr(cms::Exception("BadCallback") << "Callback executed before request was ready"));
			if (!err.IsOk()) {
				std::cout << "Could not read the result" <<  ": " << err << std::endl;
				this->output_.resize(noutput_ * batchSize_, 0.f);
			} else {
			auto t3 = std::chrono::high_resolution_clock::now();

			// std::map<std::string, ni::ModelStatus> end_status;
			// GetServerSideStatus(&end_status);

			edm::LogInfo("TRTClient") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

			//check result
				this->getResults(results.begin()->second);

			//ServerSideStats stats;
			// SummarizeServerStats(std::make_pair(modelName_, -1), start_status, end_status, &stats);
			//ReportServerSideState(stats);
			}
			//finish
			this->finish();
		});
}

template <typename Client>
void
TRTClient<Client>::ReportServerSideState(const ServerSideStats& stats)
{
	// https://github.com/NVIDIA/tensorrt-inference-server/blob/master/src/clients/c%2B%2B/perf_client/inference_profiler.cc
	const uint64_t cnt = stats.request_count;
	if (cnt == 0)
	{
		std::cout << "  Request count: " << cnt << std::endl;
		return;
	}

	const uint64_t cumm_time_us = stats.cumm_time_ns / 1000;
	const uint64_t cumm_avg_us = cumm_time_us / cnt;

	const uint64_t queue_time_us = stats.queue_time_ns / 1000;
	const uint64_t queue_avg_us = queue_time_us / cnt;

	const uint64_t compute_time_us = stats.compute_time_ns / 1000;
	const uint64_t compute_avg_us = compute_time_us / cnt;

	const uint64_t overhead = (cumm_avg_us > queue_avg_us + compute_avg_us)
								  ? (cumm_avg_us - queue_avg_us - compute_avg_us)
								  : 0;
	std::cout << "  Request count: " << cnt << std::endl
			  << "  Avg request latency: " << cumm_avg_us << " usec";

	std::cout << " (overhead " << overhead << " usec + "
				<< "queue " << queue_avg_us << " usec + "
				<< "compute " << compute_avg_us << " usec)" << std::endl
				<< std::endl;
}

template <typename Client>
void
TRTClient<Client>::SummarizeServerStats(
    const ModelInfo model_info,
    const std::map<std::string, ni::ModelStatus>& start_status,
    const std::map<std::string, ni::ModelStatus>& end_status,
    ServerSideStats* server_stats)
{
  	SummarizeServerModelStats(
      model_info.first, model_info.second,
      start_status.find(model_info.first)->second,
      end_status.find(model_info.first)->second, server_stats);

//   // Summarize the composing models, if any.
//   for (const auto& composing_model_info : composing_models_map_[model_info]) {
//     auto it = server_stats->composing_models_stat
//                   .emplace(composing_model_info, ServerSideStats())
//                   .first;
//     if (composing_models_map_.find(composing_model_info) !=
//         composing_models_map_.end()) {
//       RETURN_IF_ERROR(SummarizeServerStats(
//           composing_model_info, start_status, end_status, &(it->second)));
//     } else {
//       RETURN_IF_ERROR(SummarizeServerModelStats(
//           composing_model_info.first, composing_model_info.second,
//           start_status.find(composing_model_info.first)->second,
//           end_status.find(composing_model_info.first)->second, &(it->second)));
//     }

//   return nic::Error::Success;
}

template <typename Client>
void
TRTClient<Client>::SummarizeServerModelStats(
    const std::string& model_name, const int64_t model_version,
    const ni::ModelStatus& start_status, const ni::ModelStatus& end_status,
    ServerSideStats* server_stats)
{
  // If model_version is -1 then look in the end status to find the
  // latest (highest valued version) and use that as the version.
  int64_t status_model_version = 0;
  if (model_version < 0) {
    for (const auto& vp : end_status.version_status()) {
      status_model_version = std::max(status_model_version, vp.first);
    }
  } else {
    status_model_version = model_version;
  }

  const auto& vend_itr = end_status.version_status().find(status_model_version);
  if (vend_itr == end_status.version_status().end()) {
    return;
  } else {
    const auto& end_itr =
        vend_itr->second.infer_stats().find(batchSize_);
    if (end_itr == vend_itr->second.infer_stats().end()) {
      return;
    } else {
      uint64_t start_cnt = 0;
      uint64_t start_cumm_time_ns = 0;
      uint64_t start_queue_time_ns = 0;
      uint64_t start_compute_time_ns = 0;

      const auto& vstart_itr =
          start_status.version_status().find(status_model_version);
      if (vstart_itr != start_status.version_status().end()) {
        const auto& start_itr =
            vstart_itr->second.infer_stats().find(batchSize_);
        if (start_itr != vstart_itr->second.infer_stats().end()) {
          start_cnt = start_itr->second.success().count();
          start_cumm_time_ns = start_itr->second.success().total_time_ns();
          start_queue_time_ns = start_itr->second.queue().total_time_ns();
          start_compute_time_ns = start_itr->second.compute().total_time_ns();
        }
      }

      server_stats->request_count =
          end_itr->second.success().count() - start_cnt;
      server_stats->cumm_time_ns =
          end_itr->second.success().total_time_ns() - start_cumm_time_ns;
      server_stats->queue_time_ns =
          end_itr->second.queue().total_time_ns() - start_queue_time_ns;
      server_stats->compute_time_ns =
          end_itr->second.compute().total_time_ns() - start_compute_time_ns;
    }
  }
}

template <typename Client>
void
TRTClient<Client>::GetServerSideStatus(
    std::map<std::string, ni::ModelStatus>* model_status)
{
  model_status->clear();

  ni::ServerStatus server_status;
  server_ctx_->GetServerStatus(&server_status);
  GetServerSideStatus(
      server_status, std::make_pair(modelName_, -1), // HARDCODED model_version_ = -1
      model_status);
}

template <typename Client>
void
TRTClient<Client>::GetServerSideStatus(
    ni::ServerStatus& server_status, const ModelInfo model_info,
    std::map<std::string, ni::ModelStatus>* model_status)
{
  const auto& itr = server_status.model_status().find(model_info.first);
  if (itr == server_status.model_status().end()) {
    return;
  } else {
    model_status->emplace(model_info.first, itr->second);
  }

//   // Also get status for composing models if any
//   for (const auto& composing_model_info : composing_models_map_[model_info]) {
//     if (composing_models_map_.find(composing_model_info) !=
//         composing_models_map_.end()) {
//       GetServerSideStatus(
//           server_status, composing_model_info, model_status);
//     } else {
//       const auto& itr =
//           server_status.model_status().find(composing_model_info.first);
//       if (itr == server_status.model_status().end()) {
//         return nic::Error(
//             ni::RequestStatusCode::INTERNAL,
//             "unable to find status for composing model" +
//                 composing_model_info.first);
//       } else {
//         model_status->emplace(composing_model_info.first, itr->second);
//       }
//     }
}

//explicit template instantiations
template class TRTClient<SonicClientSync<std::vector<float>>>;
template class TRTClient<SonicClientAsync<std::vector<float>>>;
template class TRTClient<SonicClientPseudoAsync<std::vector<float>>>;
