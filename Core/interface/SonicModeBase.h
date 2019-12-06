#ifndef SonicCMS_Core_SonicModeBase
#define SonicCMS_Core_SonicModeBase

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include <string>
#include <chrono>

class SonicModeBase {
	public:
		virtual ~SonicModeBase() {}

		void setDebugName(const std::string& debugName) { debugName_ = debugName; }

		virtual void predict(edm::WaitingTaskWithArenaHolder holder) = 0;

	protected:
		virtual void predictImpl() = 0;

		void setStartTime() {
			if(debugName_.empty()) return;
			t0_ = std::chrono::high_resolution_clock::now();
			setTime_ = true;
		}

		void finish() {
			if(setTime_){
				auto t1 = std::chrono::high_resolution_clock::now();
				edm::LogInfo(debugName_) << "Client time: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0_).count();
			}
			std::exception_ptr exceptionPtr;
			holder_.doneWaiting(exceptionPtr);
		}

		//members
		edm::WaitingTaskWithArenaHolder holder_;
		//for logging/debugging
		std::string debugName_;
		std::chrono::time_point<std::chrono::high_resolution_clock> t0_;
		bool setTime_ = false;
};

#endif
