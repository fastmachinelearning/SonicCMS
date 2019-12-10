#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SonicCMS/Brainwave/interface/TFClientRemote.h"

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

void TFClientRemote::predictImpl(){
	//items for grpc request
	PredictRequest request;
	PredictResponse response;
	ClientContext context;
	CompletionQueue cq;

	//setup input
	protomap& inputs = *request.mutable_inputs();
	input_->AsProtoTensorContent(inputs["images"]);

	//setup timeout
	auto t1 = std::chrono::high_resolution_clock::now();
	std::chrono::system_clock::time_point deadline = t1 + std::chrono::seconds(timeout_);
	context.set_deadline(deadline);
		
	//make prediction request
	auto rpc = stub_->AsyncPredict(&context, request, &cq);

	//setup reply, status, tag (default to 1)
	Status status;
	rpc->Finish(&response,&status,(void*)(1));

	//wait until completion
	void* tag;
	bool ok = false;
	cq.Next(&tag,&ok);

	auto t2 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TFClientRemote") << "Remote time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		
	//check result
	std::exception_ptr exceptionPtr;
	if(ok and status.ok() and tag==(void*)(1)){
		protomap& outputs = *response.mutable_outputs();
		output_->FromProto(outputs["output_alias"]);
		std::stringstream msg;
		edm::LogInfo("TFClientRemote") << "Classifier Status: Ok\n";
	}
	else{
		edm::LogInfo("TFClientRemote") << "gRPC call return code: " << status.error_code() << ", msg: " << status.error_message();
	}
}

//based on: tensorflow_serving/example/inception_client.cc
//and also: https://github.com/Azure/aml-real-time-ai/blob/master/pythonlib/amlrealtimeai/client.py
//and also: https://github.com/grpc/grpc/blob/v1.14.1/examples/cpp/helloworld/greeter_async_client.cc
TFClientRemote::TFClientRemote(const edm::ParameterSet& params) :
	SonicClientPseudoAsync<tensorflow::Tensor>(),
	channel_(grpc::CreateChannel(
		params.getParameter<std::string>("address")+":"+std::to_string(params.getParameter<unsigned>("port")),
		grpc::InsecureChannelCredentials()
	)),
	stub_(PredictionService::NewStub(channel_)),
	timeout_(params.getParameter<unsigned>("timeout"))
{ }
