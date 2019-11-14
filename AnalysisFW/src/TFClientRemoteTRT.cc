#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SonicCMS/AnalysisFW/interface/TFClientRemoteTRT.h"

#include <sstream>
#include <chrono>
#include <thread>

namespace ni = nvidia::inferenceserver;
namespace nic = nvidia::inferenceserver::client;

JetImageData::JetImageData() :
	dataID_(0),
	timeout_(0),
	batchSize_(1),
	output_(nullptr),
	hasCall_(false),
	stop_(false)
{
	thread_ = std::make_unique<std::thread>([this](){ waitForNext(); });
	
}

JetImageData::~JetImageData() {
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

void JetImageData::waitForNext(){
	while(true){
		//wait for condition
		{
			std::unique_lock<std::mutex> lk(mutex_);
			cond_.wait(lk, [this](){return (hasCall_ or stop_);});
			if(stop_) break;

			std::unique_ptr<nic::InferContext> ctx;
			nic::InferGrpcContext::Create(&ctx,url_,modelName_,-1,false);
			std::unique_ptr<nic::InferContext::Options> options;
			nic::InferContext::Options::Create(&options);
			
			options->SetBatchSize(batchSize_);
			for (const auto& output : ctx->Outputs()) {
				options->AddRawResult(output);
			}
			ctx->SetRunOptions(*options);

			const std::vector<std::shared_ptr<nic::InferContext::Input>>& inputs = ctx->Inputs();
			std::shared_ptr<nic::InferContext::Input> input = inputs[0];
			input->Reset();

			auto t2 = std::chrono::high_resolution_clock::now();
			std::vector<int64_t> input_shape;
			input_shape.push_back(15);
			input->SetShape(input_shape);
			for(unsigned i0 = 0; i0 < batchSize_; i0++) {
				nic::Error err1 = input->SetRaw(reinterpret_cast<const uint8_t*>(&input_[0]),ninput_ * sizeof(float));
			}
			auto t3 = std::chrono::high_resolution_clock::now();
			auto time2 = std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();
			edm::LogInfo("TFClientRemoteTRT") << "Image array time: " << time2;
			std::map<std::string, std::unique_ptr<nic::InferContext::Result>> results;
			nic::Error err0 = ctx->Run(&results);
			auto t4 = std::chrono::high_resolution_clock::now();
			auto time3 = std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count();
			edm::LogInfo("TFClientRemoteTRT") << "Remote time: " << time3;

			//check result
			std::exception_ptr exceptionPtr;
			const std::unique_ptr<nic::InferContext::Result>& result = results.begin()->second;
			for(unsigned i0 = 0; i0 < batchSize_; i0++) { 
				const uint8_t* r0;
				size_t content_byte_size;
				result->GetRaw(i0, &r0,&content_byte_size);
				const float *lVal = reinterpret_cast<const float*>(r0);
				for(unsigned i1 = 0; i1 < noutput_; i1++) output_[i0*noutput_+i1] = lVal[i1]; //This should be replaced with a memcpy
			}
			auto t5 = std::chrono::high_resolution_clock::now();
			auto time4 = std::chrono::duration_cast<std::chrono::microseconds>(t5-t4).count();
			edm::LogInfo("TFClientRemoteTRT") << "Output time: " << time4;
			edm::LogInfo("TFClientRemoteTRT") << "Classifier Status: Ok\n";
			hasCall_ = false;
			holder_.doneWaiting(exceptionPtr);
		}
	}
}

//based on: tensor-rt-client simple_example
TFClientRemoteTRT::TFClientRemoteTRT(unsigned numStreams, const std::string& address, int port, unsigned timeout,const std::string& model_name,unsigned batchSize,unsigned ninput,unsigned noutput) :
	TFClientBase(),
	streamData_(numStreams),
	timeout_(timeout),
	batchSize_(batchSize),
	ninput_(ninput),
	noutput_(noutput)
{
	url_=address+":"+std::to_string(port);
	modelName_ = model_name;
}

//input is "image" in tensor form
void TFClientRemoteTRT::predict(unsigned dataID, const float* img, float* result, edm::WaitingTaskWithArenaHolder holder) {
	//get cache
	auto& streamData = streamData_.at(dataID);
	//do all read/writes inside lock to ensure cache synchronization
	{
		std::lock_guard<std::mutex> guard(streamData.mutex_);
		streamData.dataID_ = dataID;
		streamData.timeout_ = timeout_;
		streamData.batchSize_ = batchSize_;
		streamData.url_ = url_;
		streamData.modelName_ = modelName_;
		streamData.ninput_= ninput_;
		streamData.noutput_ = noutput_;
		streamData.output_= result;
		streamData.holder_= std::move(holder);
		streamData.input_ = img;
		//activate thread to wait for response, and return
		streamData.hasCall_ = true;
	}
	streamData.cond_.notify_one();
}
