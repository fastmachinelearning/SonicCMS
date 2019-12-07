#ifndef SonicCMS_Core_SonicClientSync
#define SonicCMS_Core_SonicClientSync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "SonicCMS/Core/interface/SonicClientBase.h"
#include "SonicCMS/Core/interface/SonicClientTypes.h"

template <typename InputT, typename OutputT=InputT>
class SonicClientSync : public SonicClientBase, public SonicClientTypes<InputT,OutputT> {
	public:
		virtual ~SonicClientSync() {}

		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			holder_ = std::move(holder);
			setStartTime();
			predictImpl();

			//sync Client calls holder at the end
			finish();
		}		
};

#endif

