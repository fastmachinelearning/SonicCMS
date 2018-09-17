#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SonicCMS/AnalysisFW/interface/TFClientRemote.h"

#include <sstream>
#include <chrono>

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

//based on: tensorflow_serving/example/inception_client.cc
//and also: https://github.com/Azure/aml-real-time-ai/blob/master/pythonlib/amlrealtimeai/client.py
//and also: https://github.com/grpc/grpc/blob/v1.14.1/examples/cpp/helloworld/greeter_async_client.cc
TFClientRemote::TFClientRemote(const std::string& address, int port, unsigned timeout) :
	TFClientBase(),
	channel_(grpc::CreateChannel(address+":"+std::to_string(port),grpc::InsecureChannelCredentials())),
	stub_(PredictionService::NewStub(channel_)),
	timeout_(timeout)
{ }

//input is "image" in tensor form
bool TFClientRemote::predict(const tensorflow::Tensor& img, tensorflow::Tensor& result, unsigned dataID) const {
	//convert to proto
	tensorflow::TensorProto proto;
	img.AsProtoTensorContent(&proto);
	
	//items for grpc request
	PredictRequest predictRequest;
	PredictResponse response;
	ClientContext context;
	CompletionQueue cq;
	
	//setup input
	protomap& inputs = *predictRequest.mutable_inputs();
	inputs["images"] = proto;

	//setup timeout
	std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_);
	context.set_deadline(deadline);
	
	//make prediction request
	auto rpc = stub_->AsyncPredict(&context, predictRequest, &cq);

	//setup reply, status, unique tag
	Status status;
	rpc->Finish(&response,&status,(void*)dataID);

	//block until completion
	void* tag;
	bool ok = false;
	cq.Next(&tag,&ok);

	//check result
	if(ok and status.ok() and tag==(void*)dataID){
		protomap& outputs = *response.mutable_outputs();
		result.FromProto(outputs["output_alias"]);
		std::stringstream msg;
		msg << "Classifier Status: Ok\n";
		edm::LogInfo("TFClientRemote") << msg.str();
		return true;
	}
	else{
		edm::LogInfo("TFClientRemote") << "gRPC call return code: " << status.error_code() << ", msg: " << status.error_message();
		return false;
	}
}
