#ifndef TFCLIENT_H
#define TFCLIENT_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <string>
#include <sstream>
#include <memory>
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
class TFClient {
	public:
		//constructors (timeout in seconds)
		TFClient() {}
		TFClient(const std::string& address, int port, unsigned timeout) :
			channel_(grpc::CreateChannel(address+":"+std::to_string(port),grpc::InsecureChannelCredentials())),
			stub_(PredictionService::NewStub(channel_)),
			timeout_(timeout)
		{ }
		
		void setSession(const std::string& address, int port, unsigned timeout) {
			channel_ = grpc::CreateChannel(address+":"+std::to_string(port),grpc::InsecureChannelCredentials());
			stub_ = PredictionService::NewStub(channel_);
			timeout_ = timeout;
		}
		
		//input is "image" in proto form
		bool predict(tensorflow::Tensor& img, tensorflow::Tensor& result) const {
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
/*				msg << "\n";
				msg << "output vector size = " << outputs.size() << "\n";
				msg << "output vector = " << outputs["output_alias"].DebugString() << "\n";
*/				edm::LogInfo("TFClient") << msg.str();
				return true;
			}
			else{
				edm::LogInfo("TFClient") << "gRPC call return code: " << status.error_code() << ", msg: " << status.error_message();
				return false;
			}
		}
		
		
	private:
		std::shared_ptr<Channel> channel_;
		std::unique_ptr<PredictionService::Stub> stub_;
		unsigned timeout_;
};

#endif
