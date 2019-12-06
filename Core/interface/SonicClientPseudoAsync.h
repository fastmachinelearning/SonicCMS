#ifndef SonicCMS_Core_SonicClientPseudoAsync
#define SonicCMS_Core_SonicClientPseudoAsync

#include "FWCore/Concurrency/interface/WaitingTaskWithArenaHolder.h"
#include "SonicCMS/Core/interface/SonicClientBase.h"

#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

//pretend to be async + non-blocking by waiting for blocking calls to return in separate std::thread
template <typename InputT, typename OutputT=InputT>
class SonicClientPseudoAsync : public SonicClientBase<InputT,OutputT> {
	public:
		//constructor
		SonicClientPseudoAsync() : SonicClientBase<InputT,OutputT>(), hasCall_(false), stop_(false) {
			thread_ = std::make_unique<std::thread>([this](){ waitForNext(); });
		}
		//destructor
		virtual ~SonicClientPseudoAsync() {
			{
				std::lock_guard<std::mutex> guard(mutex_);
				stop_ = true;
			}
			cond_.notify_one();
			if(thread_){
				thread_->join();
				thread_.reset();
			}
		}
		//accessor
		void predict(edm::WaitingTaskWithArenaHolder holder) override final {
			//do all read/writes inside lock to ensure cache synchronization
			{
				std::lock_guard<std::mutex> guard(mutex_);
				this->holder_ = std::move(holder);
				this->setStartTime();

				//activate thread to wait for response, and return
				hasCall_ = true;
			}
			cond_.notify_one();
		}

	protected:
		void waitForNext() {
			while(true){
				//wait for condition
				{
					std::unique_lock<std::mutex> lk(mutex_);
					cond_.wait(lk, [this](){return (hasCall_ or stop_);});
					if(stop_) break;

					//do everything inside lock
					this->predictImpl();
					
					//pseudo-async calls holder at the end (inside std::thread)
					hasCall_ = false;
					this->finish();
				}
			}
		}

		//members
		edm::WaitingTaskWithArenaHolder holder_;
		bool hasCall_;
		std::mutex mutex_;
		std::condition_variable cond_;
		std::atomic<bool> stop_;
		std::unique_ptr<std::thread> thread_;
};

#endif
