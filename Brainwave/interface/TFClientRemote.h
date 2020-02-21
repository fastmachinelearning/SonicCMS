#ifndef SonicCMS_Brainwave_TFClientRemote
#define SonicCMS_Brainwave_TFClientRemote

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClientPseudoAsync.h"

#include <memory>
#include <string>

#include "grpc++/create_channel.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/util/command_line_flags.h"

//forward declaration
namespace edm {
	class ParameterSetDescription;
}

class TFClientRemote : public SonicClientPseudoAsync<tensorflow::Tensor> {
	public:
		//constructors (timeout in seconds)
		TFClientRemote(const edm::ParameterSet& params);
		//for fillDescriptions
		static void fillPSetDescription(edm::ParameterSetDescription& iDesc);

	protected:
		void predictImpl() override;

		//members
		std::shared_ptr<grpc::Channel> channel_;
		std::unique_ptr<tensorflow::serving::PredictionService::Stub> stub_;
		unsigned timeout_;
		std::string inputTensorName_;
};

#endif
