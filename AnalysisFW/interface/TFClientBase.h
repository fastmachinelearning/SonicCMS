#ifndef TFCLIENTBASE_H
#define TFCLIENTBASE_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "tensorflow/core/framework/tensor.h"

//base class for local and remote clients
class TFClientBase {
	public:
		//constructor
		TFClientBase() {}
		//destructor
		virtual ~TFClientBase() {}
		
		//input is "image" in tensor form
		virtual void predict(unsigned dataID, const tensorflow::Tensor* img, tensorflow::Tensor* result, edm::WaitingTaskWithArenaHolder holder) {}
};

#endif
