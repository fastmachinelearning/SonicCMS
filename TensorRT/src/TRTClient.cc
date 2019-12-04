#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"

#include "request_grpc.h"

#include <string>

namespace nic = nvidia::inferenceserver::client;

//based on https://github.com/NVIDIA/tensorrt-inference-server/blob/master/src/clients/c++/examples/simple_callback_client.cc

template <typename Mode>
TRTClient<Mode>::TRTClient(const edm::ParameterSet& params) :
	SonicClient<Mode, std::vector<float>>(),
	url_(params.getParameter<std::string>("address")+":"+std::to_string(params.getParameter<unsigned>("port"))),
	timeout_(params.getParameter<unsigned>("timeout")),
	modelName_(params.getParameter<std::string>("modelName")),
	batchSize_(params.getParameter<unsigned>("batchSize")),
	ninput_(params.getParameter<unsigned>("ninput")),
	noutput_(params.getParameter<unsigned>("noutput"))
{
}

template <typename Mode>
void TRTClient<Mode>::setup() {
	nic::InferGrpcContext::Create(&context_, url_, modelName_, -1, false);
	std::unique_ptr<nic::InferContext::Options> options;
	nic::InferContext::Options::Create(&options);

	options->SetBatchSize(batchSize_);
	for (const auto& output : context_->Outputs()) {
		options->AddRawResult(output);
	}
	context_->SetRunOptions(*options);

	const std::vector<std::shared_ptr<nic::InferContext::Input>>& nicinputs = context_->Inputs();
	nicinput_ = nicinputs[0];
	nicinput_->Reset();

	std::vector<int64_t> input_shape;
	for(unsigned i0 = 0; i0 < batchSize_; i0++) {
		nic::Error err1 = nicinput_->SetRaw(reinterpret_cast<const uint8_t*>(this->input_.data()), ninput_ * sizeof(float));
	}
}

template <typename Mode>
void TRTClient<Mode>::getResults(const std::unique_ptr<nic::InferContext::Result>& result) {
	this->output_.resize(noutput_*batchSize_,0.f);
	for(unsigned i0 = 0; i0 < batchSize_; i0++) { 
		const uint8_t* r0;
		size_t content_byte_size;
		result->GetRaw(i0, &r0, &content_byte_size);
		const float *lVal = reinterpret_cast<const float*>(r0);
		for(unsigned i1 = 0; i1 < noutput_; i1++) this->output_[i0*noutput_+i1] = lVal[i1]; //This should be replaced with a memcpy
	}
}

template <typename Mode>
void TRTClient<Mode>::predictImpl(){
	//common operations first
	setup();

	//blocking call
	std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
	nic::Error err0 = context_->Run(&results);
	getResults(results.begin()->second);
}

//specialization for true async
template <>
void TRTClientAsync::predictImpl(){
	//non-blocking call
	nic::Error erro0 = context_->AsyncRun(
		[this](nic::InferContext* ctx, const std::shared_ptr<nic::InferContext::Request>& request) {
			//get results
			std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
			//this function interface will change in the next tensorrtis version
			bool is_ready = false;
			ctx->GetAsyncRunResults(&results, &is_ready, request, false);
			if(is_ready == false) throw cms::Exception("BadCallback") << "Callback executed before request was ready";

			//check result
			this->getResults(results.begin()->second);

			//finish
			this->finish();
		}
	);
}

//explicit template instantiations
template class TRTClient<SonicModeSync>;
template class TRTClient<SonicModeAsync>;
template class TRTClient<SonicModePseudoAsync>;
