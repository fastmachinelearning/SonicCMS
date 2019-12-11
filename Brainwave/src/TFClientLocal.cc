#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"
#include "SonicCMS/Brainwave/interface/TFClientLocal.h"

#include <sstream>
#include <chrono>

#include "tensorflow/core/graph/default_device.h"

TFClientLocal::TFClientLocal(const edm::ParameterSet& params) : SonicClientSync<tensorflow::Tensor>()
{
	loadModel(params.getParameter<std::string>("featurizer"), params.getParameter<std::string>("classifier"));
	createSessions();
}

void TFClientLocal::loadModel(const std::string& featurizer_file, const std::string& classifier_file) {
    // load the graph 
    std::stringstream msg;
    msg << "[loadModel] Loading the .pb files...\n";
    tensorflow::setLogging();

    graphDefFeaturizer_ = tensorflow::loadGraphDef(featurizer_file);
    msg << "featurizer node size = " << graphDefFeaturizer_->node_size() << "\n";
    // Don't print out all nodes -- it's humongous
    auto shape0F = graphDefFeaturizer_->node().Get(0).attr().at("shape").shape();
    msg << "featurizer shape0 size = " << shape0F.dim_size() << "\n";
    for (int i = 0; i < shape0F.dim_size(); i++) {
      msg << shape0F.dim(i).size() << "\n";
    }

    graphDefClassifier_ = tensorflow::loadGraphDef(classifier_file);
    msg << "classifier node size = " << graphDefClassifier_->node_size() << "\n";
    for (int i = 0; i < graphDefClassifier_->node_size(); i++) {
      msg << graphDefClassifier_->node(i).name() << "\n";
    }
    auto shape0C = graphDefClassifier_->node().Get(0).attr().at("shape").shape();
    msg << "classifier shape0 size = " << shape0C.dim_size() << "\n";
    for (int i = 0; i < shape0C.dim_size(); i++) {
      msg << shape0C.dim(i).size() << "\n";
    }
    edm::LogInfo("TFClientLocal") << msg.str();
}

void TFClientLocal::createSessions(){
    std::stringstream msg;
	auto t1 = std::chrono::high_resolution_clock::now();

	msg.str("");
    msg << "Create featurizer session...\n";
    sessionF_ = tensorflow::createSession(graphDefFeaturizer_);
    edm::LogInfo("TFClientLocal") << msg.str();

	auto t2 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TFClientLocal") << "Featurizer session time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	msg.str("");
    msg << "Create classifier session...\n";
    sessionC_ = tensorflow::createSession(graphDefClassifier_);
    edm::LogInfo("TFClientLocal") << msg.str();

	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TFClientLocal") << "Classifier session time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
}

TFClientLocal::~TFClientLocal() {
    std::stringstream msg;
	if(sessionF_){
		msg.str("");
	    msg << "Close the featurizer session..."  << "\n";
		tensorflow::closeSession(sessionF_);
	    edm::LogInfo("TFClientLocal") << msg.str();
	}
	if(sessionC_){
		msg.str("");
	    msg << "Close the classifier session..."  << "\n";
		tensorflow::closeSession(sessionC_);
	    edm::LogInfo("TFClientLocal") << msg.str();
	}
}

std::vector<tensorflow::Tensor> TFClientLocal::runFeaturizer(const tensorflow::Tensor& inputImage) const {
    std::stringstream msg;
    msg << " ====> Run the Featurizer...\n";
    // Tensorflow part
    msg << "Featurizer input = " << inputImage.DebugString() << "\n";
    edm::LogInfo("TFClientLocal") << msg.str();

    msg.str("");
    std::vector<tensorflow::Tensor> featurizer_outputs;
    tensorflow::Status statusF = sessionF_->Run( {{"InputImage:0",inputImage}}, { "resnet_v1_50/pool5:0" }, {}, &featurizer_outputs);
    if (!statusF.ok()) { msg << statusF.ToString() << "\n"; }
    else { msg << "Featurizer Status: Ok\n"; }
    edm::LogInfo("TFClientLocal") << msg.str();

    return featurizer_outputs;
}

std::vector<tensorflow::Tensor> TFClientLocal::runClassifier(const tensorflow::Tensor& inputClassifier) const {
    std::stringstream msg;
    msg << " ====> Run the Classifier...\n";
	// Tensorflow part
    msg << "Classifier input = " << inputClassifier.DebugString() << "\n";
    edm::LogInfo("TFClientLocal") << msg.str();

    msg.str("");
    std::vector<tensorflow::Tensor> outputs;
    tensorflow::Status statusC = sessionC_->Run( {{"Input:0",inputClassifier}}, { "resnet_v1_50/logits/Softmax:0" }, {}, &outputs);
    if (!statusC.ok()) { msg << statusC.ToString() << "\n"; }
    else{ msg << "Classifier Status: Ok"  << "\n"; }
    msg << "output vector size = " << outputs.size() << "\n";
    msg << "output vector = " << outputs[0].DebugString() << "\n";
    edm::LogInfo("TFClientLocal") << msg.str();

    return outputs;
}

tensorflow::Tensor TFClientLocal::createFeatureList(const tensorflow::Tensor& input) const {
    tensorflow::Tensor inputClassifier(tensorflow::DT_FLOAT, { 1, 1, 1, 2048 });
    auto input_map_classifier = inputClassifier.tensor<float,4>();
    auto feature_list = input.tensor<float,4>();
    for (int itf = 0; itf < 2048; itf++){
      input_map_classifier(0,0,0,itf) = feature_list(0,0,0,itf);
    }
    return inputClassifier;
}

//input is "image" in tensor form
void TFClientLocal::predictImpl() {
	auto t1 = std::chrono::high_resolution_clock::now();

	// --------------------------------------------------------------------
	// Run the Featurizer
	const auto& featurizer_outputs = runFeaturizer(input_);

	auto t2 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TFClientLocal") << "Featurizer time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	// --------------------------------------------------------------------
	// Run the Classifier
	const auto& inputClassifier = createFeatureList(featurizer_outputs[0]);
	const auto& outputs = runClassifier(inputClassifier);
	output_ = outputs[0];

	auto t3 = std::chrono::high_resolution_clock::now();
	edm::LogInfo("TFClientLocal") << "Classifier time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
}
