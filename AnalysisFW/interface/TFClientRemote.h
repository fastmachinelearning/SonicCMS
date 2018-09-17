#ifndef TFCLIENTREMOTE_H
#define TFCLIENTREMOTE_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFClientBase.h"

#include <string>
#include <memory>

#include "grpc++/create_channel.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/util/command_line_flags.h"

class TFClientRemote : public TFClientBase {
	public:
		//constructors (timeout in seconds)
		TFClientRemote() : TFClientBase() {}
		TFClientRemote(const std::string& address, int port, unsigned timeout);
		
		//input is "image" in tensor form
		bool predict(const tensorflow::Tensor& img, tensorflow::Tensor& result) const override;
		
	private:
		std::shared_ptr<grpc::Channel> channel_;
		std::unique_ptr<tensorflow::serving::PredictionService::Stub> stub_;
		unsigned timeout_;
};

#endif
