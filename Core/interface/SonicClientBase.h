#ifndef SonicCMS_Core_SonicClientBase
#define SonicCMS_Core_SonicClientBase

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"

#include <string>
#include <chrono>

template <typename InputT, typename OutputT=InputT>
class SonicClientBase {
	public:
		//typedefs for outside accessibility
		typedef InputT Input;
		typedef OutputT Output;
		//destructor
		virtual ~SonicClientBase() {}

		void setDebugName(const std::string& debugName) { debugName_ = debugName; }

		//accessors
		const Input& input() const { return input_; }
		void setInput(const Input& inp) { input_ = inp; }
		const Output& output() const { return output_; }

		//main operation
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
		Input input_;
		Output output_;
		edm::WaitingTaskWithArenaHolder holder_;

		//for logging/debugging
		std::string debugName_;
		std::chrono::time_point<std::chrono::high_resolution_clock> t0_;
		bool setTime_ = false;
};

#endif
