#ifndef SonicCMS_Core_SonicModeSync
#define SonicCMS_Core_SonicModeSync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

class SonicModeSync {
	public:
		virtual ~SonicModeSync() {}
		
		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) {
			predictImpl();
			
			//sync mode calls holder at the end
			std::exception_ptr exceptionPtr;
			holder.doneWaiting(exceptionPtr);
		}		
		
	protected:
		//sync mode does not pass holder to impl
		virtual void predictImpl() = 0;
};

#endif

