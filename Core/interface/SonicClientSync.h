#ifndef SonicCMS_Core_SonicClientSync
#define SonicCMS_Core_SonicClientSync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "SonicCMS/Core/interface/SonicClientBase.h"

template <typename InputT, typename OutputT=InputT>
class SonicClientSync : public SonicClientBase<InputT,OutputT> {
	public:
		virtual ~SonicClientSync() {}

		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			this->holder_ = std::move(holder);
			this->setStartTime();
			this->predictImpl();

			//sync Client calls holder at the end
			this->finish();
		}		
};

#endif

