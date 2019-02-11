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
	dataID_(0),
	timeout_(0),
	stub_(nullptr),
	output_(nullptr),
	hasCall_(false),
	stop_(false)
{
	thread_ = std::make_unique<std::thread>([this](){ waitForNext(); });
}

JetImageData::~JetImageData() {
	{
		std::lock_guard<std::mutex> guard(mutex_);
		stop_ = true;
	}
	cond_.notify_one();
	if(thread_){
		thread_->join();
		thread_.reset();
	}
}

void JetImageData::waitForNext(){
	while(true){
		//wait for condition
		{
			std::unique_lock<std::mutex> lk(mutex_);
			cond_.wait(lk, [this](){return (hasCall_ or stop_);});
			if(stop_) break;

			//do everything inside lock

			//items for grpc request
			PredictRequest request;
			PredictResponse response;
			ClientContext context;
			CompletionQueue cq;

			//setup input
			protomap& inputs = *request.mutable_inputs();
			inputs["images"] = proto_;

			//setup timeout
			auto t1 = std::chrono::high_resolution_clock::now();
			std::chrono::system_clock::time_point deadline = t1 + std::chrono::seconds(timeout_);
			context.set_deadline(deadline);
		
			//make prediction request
			auto rpc = stub_->AsyncPredict(&context, request, &cq);

			//setup reply, status, unique tag (+1 b/c dataID starts at 0)
			Status status;
			rpc->Finish(&response,&status,(void*)(dataID_+1));

			//wait until completion
			void* tag;
			bool ok = false;
			cq.Next(&tag,&ok);

			auto t2 = std::chrono::high_resolution_clock::now();
			edm::LogInfo("TFClientRemote") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		
			//check result
			std::exception_ptr exceptionPtr;
			if(ok and status.ok() and tag==(void*)(dataID_+1)){
				protomap& outputs = *response.mutable_outputs();
				output_->FromProto(outputs["output_alias"]);
				std::stringstream msg;
				edm::LogInfo("TFClientRemote") << "Classifier Status: Ok\n";
			}
			else{
				edm::LogInfo("TFClientRemote") << "gRPC call return code: " << status.error_code() << ", msg: " << status.error_message();
			}
			hasCall_ = false;
			holder_.doneWaiting(exceptionPtr);
		}
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

	//do all read/writes inside lock to ensure cache synchronization
	{
		std::lock_guard<std::mutex> guard(streamData.mutex_);
		streamData.stub_ = stub_.get();
		streamData.dataID_ = dataID;
		streamData.timeout_ = timeout_;
		streamData.output_ = result;
		streamData.holder_ = std::move(holder);
	
		//convert to proto
		img->AsProtoTensorContent(&streamData.proto_);
	
		//activate thread to wait for response, and return
		streamData.hasCall_ = true;
	}
	streamData.cond_.notify_one();
}
