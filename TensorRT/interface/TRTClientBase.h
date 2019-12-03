#ifndef SonicCMS_TensorRT_TRTClientBase
#define SonicCMS_TensorRT_TRTClientBase

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClient.h"

#include "request_grpc.h"

#include <string>

namespace nic = nvidia::inferenceserver::client;

class TRTClientBase {
	public:
		//constructors (timeout in seconds)
		TRTClientBase(const edm::ParameterSet& params);

		//helper
		void getResults(const std::unique_ptr<nic::InferContext::Result>& result);

		//accessors
		unsigned ninput() const { return ninput_; }
		unsigned noutput() const { return noutput_; }
		unsigned batchSize() const { return batchSize_; }

	protected:
		//helper for common ops
		void setup();

		//members
		std::string url_;
		unsigned timeout_;
		std::string modelName_;
		unsigned batchSize_;
		unsigned ninput_;
		unsigned noutput_;
		std::unique_ptr<nic::InferContext>* context_;
		std::shared_ptr<nic::InferContext::Input>* nicinput_; 
};

#endif
