#ifndef TFCLIENTLOCAL_H
#define TFCLIENTLOCAL_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFClientBase.h"

#include <string>
#include <vector>
#include <memory>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"

class TFClientLocal : public TFClientBase {
	public:
		//constructors (timeout in seconds)
		TFClientLocal() : TFClientBase() {}
		TFClientLocal(const std::string& featurizer_file, const std::string& classifier_file);
		
		//input is "image" in tensor form
		bool predict(const tensorflow::Tensor& img, tensorflow::Tensor& result) const override;
		
	private:
		void loadModel(const std::string& featurizer_file, const std::string& classifier_file);
		std::vector<tensorflow::Tensor> runFeaturizer(const tensorflow::Tensor& inputImage) const;
		std::vector<tensorflow::Tensor> runClassifier(const tensorflow::Tensor& inputClassifier) const;
		tensorflow::Tensor createFeatureList(const tensorflow::Tensor& input) const;
		
		//members
		tensorflow::GraphDef* graphDefFeaturizer_;
		tensorflow::GraphDef* graphDefClassifier_;
};

#endif
