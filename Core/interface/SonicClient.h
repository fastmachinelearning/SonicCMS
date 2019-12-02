#ifndef SonicCMS_Core_SonicClient
#define SonicCMS_Core_SonicClient

#include "FWCore/ParameterSet/interface/ParameterSet.h"

template <typename Mode, typename InputT, typename OutputT=InputT>
class SonicClient : public Mode {
	public:
		//typedefs for outside accessibility
		typedef InputT Input;
		typedef OutputT Output;
		//constructor
		SonicClient() : Mode() {}
		//destructor
		virtual ~SonicClient() {}
		
		//accessors
		const Input& input() const { return input_; }
		void setInput(const Input& inp) { input_ = inp; }
		const Output& output() const { return output_; }
		
		//main operation predict() inherited from Mode

	protected:
		Input input_;
		Output output_;
};

#endif
