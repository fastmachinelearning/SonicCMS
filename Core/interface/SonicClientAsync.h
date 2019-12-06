#ifndef SonicCMS_Core_SonicClientAsync
#define SonicCMS_Core_SonicClientAsync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "SonicCMS/Core/interface/SonicClientBase.h"

template <typename InputT, typename OutputT=InputT>
class SonicClientAsync : public SonicClientBase<InputT,OutputT> {
	public:
		virtual ~SonicClientAsync() {}

		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			this->holder_ = std::move(holder);
			this->setStartTime();
			this->predictImpl();
			//impl calls finish() which calls holder_
		}
};

#endif

