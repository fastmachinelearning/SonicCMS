#ifndef TFCLIENTREMOTE_H
#define TFCLIENTREMOTE_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFClientBase.h"
#include "request_grpc.h"

#include <string>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

namespace ni = nvidia::inferenceserver;
namespace nic = nvidia::inferenceserver::client;

class ClientData {
	public:
		ClientData();
		~ClientData();
		
		unsigned dataID_;
		unsigned timeout_;
		unsigned batchSize_;
		std::unique_ptr<nic::InferContext> *context_;
		std::shared_ptr<nic::InferContext::Input>* nicinput_; 
		std::string modelName_;
		std::string url_;
		unsigned ninput_;
		unsigned noutput_;
		const float *input_;
		float *output_;
		bool async_;
		edm::WaitingTaskWithArenaHolder holder_;

		std::mutex mutex_;

		//launch async w/ callback
		void predict();
};

class TFClientRemoteTRT : public TFClientBase {
	public:
		//constructors (timeout in seconds)
		TFClientRemoteTRT() : TFClientBase() {}
                TFClientRemoteTRT(unsigned numStreams, const std::string& address, int port, unsigned timeout,const std::string& model_name, unsigned batchSize, unsigned ninput, unsigned noutput,bool async);
		
		//input is "image" in tensor form
		void predict(unsigned dataID, const float* img, float* result, edm::WaitingTaskWithArenaHolder holder);
		
	private:
		std::vector<ClientData> streamData_;
		unsigned timeout_;
		unsigned batchSize_;
		std::string url_;
		std::string modelName_;
		unsigned ninput_;
		unsigned noutput_;
		bool async_;
		std::unique_ptr<nic::InferContext> *context_;
		std::shared_ptr<nic::InferContext::Input>* nicinput_; 
};

#endif
