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
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;

typedef google::protobuf::Map<std::string, tensorflow::TensorProto> protomap;

//based on: tensorflow_serving/example/inception_client.cc
//and also: https://github.com/Azure/aml-real-time-ai/blob/master/pythonlib/amlrealtimeai/client.py
TFClientRemote::TFClientRemote(const std::string& address, int port, unsigned timeout) :
	TFClientBase(),
	channel_(grpc::CreateChannel(address+":"+std::to_string(port),grpc::InsecureChannelCredentials())),
	stub_(PredictionService::NewStub(channel_)),
	timeout_(timeout)
{ }

//input is "image" in tensor form
bool TFClientRemote::predict(const tensorflow::Tensor& img, tensorflow::Tensor& result) const {
	//convert to proto
	tensorflow::TensorProto proto;
	img.AsProtoTensorContent(&proto);
	
	//items for grpc request
	PredictRequest predictRequest;
	PredictResponse response;
	ClientContext context;
	
	//setup input
	protomap& inputs = *predictRequest.mutable_inputs();
	inputs["images"] = proto;
	
	//setup timeout
	std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_);
	context.set_deadline(deadline);
	
	//make prediction request
	Status status = stub_->Predict(&context, predictRequest, &response);
	
	//check result
	if(status.ok()){
		protomap& outputs = *response.mutable_outputs();
		result.FromProto(outputs["output_alias"]);
		std::stringstream msg;
		msg << "Classifier Status: Ok\n";
		edm::LogInfo("TFClient") << msg.str();
		return true;
	}
	else{
		edm::LogInfo("TFClient") << "gRPC call return code: " << status.error_code() << ", msg: " << status.error_message();
		return false;
	}
}
