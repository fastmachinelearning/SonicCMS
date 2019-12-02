#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "SonicCMS/TensorRT/interface/TRTClient.h"

#include "request_grpc.h"

namespace nic = nvidia::inferenceserver::client;

//based on https://github.com/NVIDIA/tensorrt-inference-server/blob/master/src/clients/c++/examples/simple_callback_client.cc
template <typename Mode>
void TRTClient<Mode>::predict(){
	//common operations first
	setup();

	//blocking call
	std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
	nic::Error err0 = context_->Run(&results);
	getResults(results.begin()->second);
}

void TRTClientAsync::predict(edm::WaitingTaskWithArenaHolder holder){
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
			std::exception_ptr exceptionPtr;
			this->holder_.doneWaiting(exceptionPtr);
		}
	);
}
