#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SonicCMS/AnalysisFW/interface/TFClientRemote.h"

#include <sstream>
#include <chrono>
#include <thread>

#include "grpc++/create_channel.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/util/command_line_flags.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;

typedef google::protobuf::Map<std::string, tensorflow::TensorProto> protomap;

JetImageData::JetImageData() :
	output_(nullptr),
	hasCall_(false),
	stop_(false)
{
	thread_ = std::make_unique<std::thread>([this](){ waitForNext(); });
}

JetImageData::~JetImageData() {
	stop_ = true;
	if(thread_){
		thread_->join();
		thread_.reset();
	}
}

void JetImageData::predict(tensorflow::serving::PredictionService::Stub* stub, unsigned timeout, const tensorflow::Tensor* img) {
	//convert to proto
	tensorflow::TensorProto proto;
	img->AsProtoTensorContent(&proto);
	
	//setup input
	protomap& inputs = *request_.mutable_inputs();
	inputs["images"] = proto;

	//setup timeout
	grpc::ClientContext context;
	std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout);
	context.set_deadline(deadline);
	
	//make prediction request
	rpc_ = stub->AsyncPredict(&context, request_, &cq_);

	//setup reply, status, unique tag (+1 b/c dataID starts at 0)
	rpc_->Finish(&response_,&status_,(void*)(dataID_+1));
	
	//activate thread to wait for response, and return
	{
		std::lock_guard<std::mutex> guard(mutex_);
		hasCall_ = true;
	}
	cond_.notify_one();
}

void JetImageData::waitForNext(){
	while(!stop_){
		//wait for condition
		{
			std::unique_lock<std::mutex> lk(mutex_);
			cond_.wait(lk, [this](){return hasCall_;});
			lk.unlock();
		}
		
		//wait until completion
		void* tag;
		bool ok = false;
		cq_.Next(&tag,&ok);

		//check result
		std::exception_ptr exceptionPtr;
		if(ok and status_.ok() and tag==(void*)(dataID_+1)){
			protomap& outputs = *response_.mutable_outputs();
			output_->FromProto(outputs["output_alias"]);
			std::stringstream msg;
			edm::LogInfo("TFClientRemote") << "Classifier Status: Ok\n";
		}
		else{
			edm::LogInfo("TFClientRemote") << "gRPC call return code: " << status_.error_code() << ", msg: " << status_.error_message();
		}
		hasCall_ = false;
		holder_.doneWaiting(exceptionPtr);
	}
}

//based on: tensorflow_serving/example/inception_client.cc
//and also: https://github.com/Azure/aml-real-time-ai/blob/master/pythonlib/amlrealtimeai/client.py
//and also: https://github.com/grpc/grpc/blob/v1.14.1/examples/cpp/helloworld/greeter_async_client.cc
TFClientRemote::TFClientRemote(unsigned numStreams, const std::string& address, int port, unsigned timeout) :
	TFClientBase(),
	streamData_(numStreams),
	channel_(grpc::CreateChannel(address+":"+std::to_string(port),grpc::InsecureChannelCredentials())),
	stub_(PredictionService::NewStub(channel_)),
	timeout_(timeout)
{ }

//input is "image" in tensor form
void TFClientRemote::predict(unsigned dataID, const tensorflow::Tensor* img, tensorflow::Tensor* result, edm::WaitingTaskWithArenaHolder holder) {
	//get cache
	auto& streamData = streamData_.at(dataID);
	streamData.dataID_ = dataID;
	streamData.output_ = result;
	streamData.holder_ = std::move(holder);
	streamData.predict(stub_.get(),timeout_,img);
}
