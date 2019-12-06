#ifndef SonicCMS_Core_SonicModeSync
#define SonicCMS_Core_SonicModeSync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include "SonicCMS/Core/interface/SonicModeBase.h"

class SonicModeSync : public SonicModeBase {
	public:
		virtual ~SonicModeSync() {}

		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			holder_ = std::move(holder);
			setStartTime();
			predictImpl();

			//sync mode calls holder at the end
			finish();
		}		
};

#endif

