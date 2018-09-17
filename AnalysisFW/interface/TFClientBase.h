#ifndef TFCLIENTBASE_H
#define TFCLIENTBASE_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "tensorflow/core/framework/tensor.h"

//base class for local and remote clients
class TFClientBase {
	public:
		//constructor
		TFClientBase() {}
		//destructor
		virtual ~TFClientBase() {}
		
		//input is "image" in tensor form
		virtual bool predict(const tensorflow::Tensor& img, tensorflow::Tensor& result, unsigned dataID) const {
			return true;
		}
};

#endif
