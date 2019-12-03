#ifndef SonicCMS_Core_SonicModeAsync
#define SonicCMS_Core_SonicModeAsync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

class SonicModeAsync {
	public:
		virtual ~SonicModeAsync() {}
		
		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) {
			holder_ = std::move(holder);
			predictImpl();
			//impl calls finish() which calls holder_
		}
		
	protected:
		//this function must call finish()
		virtual void predictImpl() = 0;

		void finish() {
			std::exception_ptr exceptionPtr;
			holder_.doneWaiting(exceptionPtr);
		}

		//members
		edm::WaitingTaskWithArenaHolder holder_;
};

#endif

