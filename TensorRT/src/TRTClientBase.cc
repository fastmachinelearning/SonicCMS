#include "SonicCMS/TensorRT/interface/TRTClientBase.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "request_grpc.h"

namespace nic = nvidia::inferenceserver::client;

TRTClientBase::TRTClientBase(const edm::ParameterSet& params) :
	url_(params.getParameter<string>("address")+":"+std::to_string(params.getParameter<unsigned>("port"))),
	timeout_(params.getParameter<unsigned>("timeout")),
	modelName_(params.getParameter<string>("modelName")),
	batchSize_(params.getParameter<unsigned>("batchSize")),
	ninput_(params.getParameter<unsigned>("ninput")),
	noutput_(params.getParameter<unsigned>("noutput"))
{
}

void TRTClientBase::setup() {
	nic::InferGrpcContext::Create(&context_, url_, modelName_, -1, false);
	std::unique_ptr<nic::InferContext::Options> options;
	nic::InferContext::Options::Create(&options);

	options->SetBatchSize(batchSize_);
	for (const auto& output : ctx->Outputs()) {
		options->AddRawResult(output);
	}
	ctx->SetRunOptions(*options);

	const std::vector<std::shared_ptr<nic::InferContext::Input>>& nicinputs = ctx->Inputs();
	nicinput_ = nicinputs[0];
	nicinput_->Reset();

	std::vector<int64_t> input_shape;
	for(unsigned i0 = 0; i0 < batchSize_; i0++) {
		nic::Error err1 = input->SetRaw(reinterpret_cast<const uint8_t*>(input_.data()), ninput_ * sizeof(float));
	}
}

void TRTClientBase::getResults(const std::unique_ptr<nic::InferContext::Result>& result) {
	const std::unique_ptr<nic::InferContext::Result>& result = results.begin()->second;
	output_.resize(noutput_*batchSize_,0.f);
	for(unsigned i0 = 0; i0 < batchSize_; i0++) { 
		const uint8_t* r0;
		size_t content_byte_size;
		result->GetRaw(i0, &r0, &content_byte_size);
		const float *lVal = reinterpret_cast<const float*>(r0);
		for(unsigned i1 = 0; i1 < noutput_; i1++) output_[i0*noutput_+i1] = lVal[i1]; //This should be replaced with a memcpy
	}
}
