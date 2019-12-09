# SonicCMS core infrastructure

## For analyzers

The `SonicEDProducer` class template extends the basic Stream producer module in CMSSW.

To implement a concrete derived producer class, the following skeleton can be used:
```cpp
#include "SonicCMS/Core/interface/SonicEDProducer.h"
#include "FWCore/Framework/interface/MakerMacros.h"

class MyProducer : public SonicEDProducer<Client>
{
	public:
		explicit MyProducer(edm::ParameterSet const& cfg) : SonicEDProducer<Client>(cfg) {
			//for debugging
			setDebugName("MyProducer");
		}
		void acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) override {
			//convert event data to client input format
		}
		void produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) override {
			//convert client output to event data format
		}
};

DEFINE_FWK_MODULE(MyProducer);
```

The generic `Client` must be replaced with a concrete client (see next section), which has specific input and output types.

The python configuration for the producer should include a dedicated `PSet` for the client parameters:
```python
process.MyProducer = cms.EDProducer("MyProducer",
    Client = cms.PSet(
        # necessary client options go here
    )
)
```

Example producers can be found in the `plugins` folders of the other packages in this repository.

## For developers

To add a new communication protocol for SONIC, follow these steps:
1. Make a new package (directory) in the repository
2. Add a setup script for the communication protocol software and its dependencies
3. Set up the concrete client(s) that use the communication protocol
4. Add a test producer (see above) to make sure it works

To implement a concrete client, the following skeleton can be used for the `.h` file, with the function implementations in an associated `.cc` file:
```cpp
#ifndef SonicCMS_MyPackage_MyClient
#define SonicCMS_MyPackage_MyClient

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SonicCMS/Core/interface/SonicClient*.h"

class MyClient : public SonicClient*<Input,Output> {
	public:
		MyClient(const edm::ParameterSet& params);

	protected:
		void predictImpl() override;
};

#endif
```

The generic `SonicClient*` should be replaced with one of the available modes:
* `SonicClientSync`: synchronous call, blocks until the result is returned.
* `SonicClientAsync`: asynchronous, non-blocking call.
* `SonicClientPseudoAsync`: turns a synchronous, blocking call into an asynchronous, non-blocking call, by waiting for the result in a separate `std::thread`.

`SonicClientAsync` is the most efficient, but can only be used if asynchronous, non-blocking calls are supported by the communication protocol in use.

In addition, as indicated, the input and output data types must be specified.
(If both types are the same, only the input type needs to be specified.)

Example client code can be found in the `interface` and `src` directories of the other packages in this repository.
