#ifndef SonicCMS_Core_SonicModeAsync
#define SonicCMS_Core_SonicModeAsync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "SonicCMS/Core/interface/SonicModeBase.h"

class SonicModeAsync : public SonicModeBase {
	public:
		virtual ~SonicModeAsync() {}

		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			holder_ = std::move(holder);
			setStartTime();
			predictImpl();
			//impl calls finish() which calls holder_
		}
};

#endif

