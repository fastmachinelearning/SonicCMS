#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "SonicCMS/TensorRT/interface/TRTClientFPGA.h"

#include "request_grpc.h"

#include <string>
#include <chrono>
#include <exception>

namespace nic = nvidia::inferenceserver::client;

//based on https://github.com/NVIDIA/tensorrt-inference-server/blob/master/src/clients/c++/examples/simple_callback_client.cc

template <typename Client>
TRTClientFPGA<Client>::TRTClientFPGA(const edm::ParameterSet& params) :
	Client(),
	url_(params.getParameter<std::string>("address")+":"+std::to_string(params.getParameter<unsigned>("port"))),
	timeout_(params.getParameter<unsigned>("timeout")),
	modelName_(params.getParameter<std::string>("modelName")),
	batchSize_(params.getParameter<unsigned>("batchSize")),
	ninput_(params.getParameter<unsigned>("ninput")),
	noutput_(params.getParameter<unsigned>("noutput"))
{
  fSetup = false;
}

template <typename Client>
void TRTClientFPGA<Client>::setup() {
  if(!fSetup) { 
    auto err = nic::InferGrpcContext::Create(&context_, url_, modelName_, -1, false);
    if(!err.IsOk()) throw cms::Exception("BadGrpc") << "unable to create inference context: " << err;
    std::unique_ptr<nic::InferContext::Options> options;
    nic::InferContext::Options::Create(&options);
    
    options->SetBatchSize(batchSize_);
    for (const auto& output : context_->Outputs()) {
      options->AddRawResult(output);
    }
    context_->SetRunOptions(*options);
    fSetup = true;
  }
  const std::vector<std::shared_ptr<nic::InferContext::Input>>& nicinputs = context_->Inputs();
  nicinput_ = nicinputs[0];
  nicinput_->Reset();
  
  auto t2 = std::chrono::high_resolution_clock::now();
  for(unsigned i0 = 0; i0 < batchSize_; i0++) {
   nic::Error err1 = nicinput_->SetRaw(reinterpret_cast<const uint8_t*>(&(this->input_[i0*ninput_])), ninput_ * sizeof(unsigned short));
   //nic::Error err1 = nicinput_->SetRaw(reinterpret_cast<const uint8_t*>(this->input_.data()), ninput_ * sizeof(unsigned short));
  }
  //nic::Error err1 = nicinput_->SetRaw(reinterpret_cast<const uint8_t*>(this->input_.data()), batchSize_*ninput_ * sizeof(unsigned short));
  auto t3 = std::chrono::high_resolution_clock::now();
  edm::LogInfo("TRTClientFPGA") << "Image array time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();
}

template <typename Client>
void TRTClientFPGA<Client>::getResults(const std::unique_ptr<nic::InferContext::Result>& result) {
	auto t2 = std::chrono::high_resolution_clock::now();
	unsigned short tmp=0;
	this->output_.resize(noutput_*batchSize_,tmp);
	//for(unsigned i0 = 0; i0 < batchSize_; i0++) { 
	const uint8_t* r0;
	size_t content_byte_size;
	result->GetRaw(0, &r0, &content_byte_size);
	const unsigned int *lVal = reinterpret_cast<const unsigned int*>(r0);
	memcpy(this->output_.data(),&lVal[0],content_byte_size*batchSize_);
	//for(unsigned i1 = 0; i1 < noutput_; i1++) this->output_[i0*noutput_+i1] = lVal[i1]; //This should be replaced with a memcpy
	//}
	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TRTClientFPGA") << "Output time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();
}

template <typename Client>
void TRTClientFPGA<Client>::predictImpl(){
	//common operations first
	setup();

	//blocking call
	auto t2 = std::chrono::high_resolution_clock::now();
	std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
	nic::Error err0 = context_->Run(&results);
	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TRTClientFPGA") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();
	getResults(results.begin()->second);
}

//specialization for true async
template <>
void TRTClientFPGAAsync::predictImpl(){
	//common operations first
	try {
		setup();
	}
	catch (...) {
		finish(std::current_exception());
		return;
	}

	//non-blocking call
	auto t2 = std::chrono::high_resolution_clock::now();
	nic::Error erro0 = context_->AsyncRun(
		[t2,this](nic::InferContext* ctx, const std::shared_ptr<nic::InferContext::Request>& request) {
			//get results
			std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
			//this function interface will change in the next tensorrtis version
			bool is_ready = false;
			ctx->GetAsyncRunResults(&results, &is_ready, request, false);
			if(is_ready == false) finish(std::make_exception_ptr(cms::Exception("BadCallback") << "Callback executed before request was ready"));

			auto t3 = std::chrono::high_resolution_clock::now();
			edm::LogInfo("TRTClientFPGA") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();

			//check result
			this->getResults(results.begin()->second);

			//finish
			this->finish();
		}
	);
}

//explicit template instantiations
template class TRTClientFPGA<SonicClientSync<std::vector<unsigned short>>>;
template class TRTClientFPGA<SonicClientAsync<std::vector<unsigned short>>>;
template class TRTClientFPGA<SonicClientPseudoAsync<std::vector<unsigned short>>>;
