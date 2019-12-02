#ifndef SonicCMS_Core_SonicModeAsync
#define SonicCMS_Core_SonicModeAsync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

class SonicModeAsync {
	public:
		virtual ~SonicModeAsync() {}
		
		//main operation
		void predict(edm::WaitingTaskWithArenaHolder holder) {
			predictImpl(std::move(holder));
			//impl calls holder
		}		
		
	protected:
		//async mode passes holder to impl
		virtual void predictImpl(edm::WaitingTaskWithArenaHolder holder) = 0;
};

#endif

