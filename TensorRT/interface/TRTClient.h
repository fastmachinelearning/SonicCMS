#ifndef SonicCMS_TensorRT_TRTClient
#define SonicCMS_TensorRT_TRTClient

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClient.h"
#include "SonicCMS/Core/interface/SonicModeSync.h"
#include "SonicCMS/Core/interface/SonicModePseudoAsync.h"
#include "SonicCMS/Core/interface/SonicModeAsync.h"
#include "SonicCMS/TensorRT/interface/TRTClientBase.h"

template <typename Mode>
class TRTClient : public TRTClientBase, public SonicClient<Mode> {
	public:
		//constructor
		TRTClient(const edm::ParameterSet& params) : TRTClientBase(params), SonicClient<Mode>() {}

	protected:
		void predictImpl() override;
};
typedef TRTClientSync TRTClient<SonicModeSync>;
typedef TRTClientPseudoAsync TRTClient<SonicModePseudoAsync>;

class TRTClientAsync : public TRTClientBase, public SonicClient<SonicModeAsync> {
	public:
		//constructor
		TRTClient(const edm::ParameterSet& params) : TRTClientBase(params), SonicClient<SonicModeAsync>() {}

	protected:
		//different interface for true async
		void predictImpl(edm::WaitingTaskWithArenaHolder holder) override;
};

#endif
