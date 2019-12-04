#ifndef SonicCMS_Core_SonicModeBase
#define SonicCMS_Core_SonicModeBase

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

//todo: add setStartTime() for debugging, keep track in finish() and emit LogInfo

class SonicModeBase {
	public:
		virtual ~SonicModeBase() {}

		virtual void predict(edm::WaitingTaskWithArenaHolder holder) = 0;

	protected:
		virtual void predictImpl() = 0;

		void finish() {
			std::exception_ptr exceptionPtr;
			holder_.doneWaiting(exceptionPtr);
		}

		//members
		edm::WaitingTaskWithArenaHolder holder_;
};

#endif
