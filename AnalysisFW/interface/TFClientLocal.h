#ifndef TFCLIENTLOCAL_H
#define TFCLIENTLOCAL_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFClientBase.h"

#include <string>
#include <vector>
#include <memory>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/public/session.h"

class TFClientLocal : public TFClientBase {
	public:
		//constructors (timeout in seconds)
		TFClientLocal() : TFClientBase() {}
		TFClientLocal(unsigned numStreams, const std::string& featurizer_file, const std::string& classifier_file);
		~TFClientLocal();
		
		//input is "image" in tensor form
		void predict(unsigned dataID, const tensorflow::Tensor* img, tensorflow::Tensor* result, edm::WaitingTaskWithArenaHolder holder) override;
		
	private:
		void loadModel(const std::string& featurizer_file, const std::string& classifier_file);
		void createSessions(unsigned dataID);
		std::vector<tensorflow::Tensor> runFeaturizer(const tensorflow::Tensor& inputImage, tensorflow::Session* sessionF) const;
		std::vector<tensorflow::Tensor> runClassifier(const tensorflow::Tensor& inputClassifier, tensorflow::Session* sessionC) const;
		tensorflow::Tensor createFeatureList(const tensorflow::Tensor& input) const;
		
		//members
		tensorflow::GraphDef* graphDefFeaturizer_ = nullptr;
		tensorflow::GraphDef* graphDefClassifier_ = nullptr;
		std::vector<tensorflow::Session*> sessionF_ = {};
		std::vector<tensorflow::Session*> sessionC_ = {};
};

#endif
