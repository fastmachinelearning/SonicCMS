#ifndef TFCLIENTREMOTE_H
#define TFCLIENTREMOTE_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFClientBase.h"

#include <string>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "grpc++/create_channel.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/util/command_line_flags.h"

class JetImageData {
	public:
		JetImageData();
		~JetImageData();
		
		unsigned dataID_;
		unsigned timeout_;
		tensorflow::serving::PredictionService::Stub* stub_;
		tensorflow::TensorProto proto_;
		tensorflow::Tensor* output_;
		edm::WaitingTaskWithArenaHolder holder_;

		//thread to wait for completion of async call
		bool hasCall_;
		std::mutex mutex_;
		std::condition_variable cond_;

	private:
		void waitForNext();
		
		bool stop_;
		std::unique_ptr<std::thread> thread_;
};

class TFClientRemote : public TFClientBase {
	public:
		//constructors (timeout in seconds)
		TFClientRemote() : TFClientBase() {}
		TFClientRemote(unsigned numStreams, const std::string& address, int port, unsigned timeout);
		
		//input is "image" in tensor form
		void predict(unsigned dataID, const tensorflow::Tensor* img, tensorflow::Tensor* result, edm::WaitingTaskWithArenaHolder holder) override;
		
	private:
		std::vector<JetImageData> streamData_;
		std::shared_ptr<grpc::Channel> channel_;
		std::unique_ptr<tensorflow::serving::PredictionService::Stub> stub_;
		unsigned timeout_;
};

#endif
