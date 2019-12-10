#ifndef SonicCMS_Brainwave_TFClientRemote
#define SonicCMS_Brainwave_TFClientRemote

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClientPseudoAsync.h"

#include <memory>

#include "grpc++/create_channel.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/util/command_line_flags.h"

class TFClientRemote : public SonicClientPseudoAsync<tensorflow::Tensor> {
	public:
		//constructors (timeout in seconds)
		TFClientRemote(const edm::ParameterSet& params);

	protected:
		void predictImpl() override;

		//members
		std::shared_ptr<grpc::Channel> channel_;
		std::unique_ptr<tensorflow::serving::PredictionService::Stub> stub_;
		unsigned timeout_;
};

#endif
