Remote mode:
```
cd python
cmsRun HcalTest_mc_cfg.py maxEvents=25 port=<port> address=<server address> batchsize=16000 modelname=facile  maxEvents=10
```

All the different client options can be tested with an additional argument:
`mode=Async` (default), `mode=Sync`, `mode=PseudoAsync`.

Other available servers:
* `prp-gpu-1.t2.ucsd.edu`

## Timing
Some timing data will be recorded in `SonicCMS/TensorRT/python/data`. The most interesting timing data is stored in `client-data.dat`. Some parts of `TRTClient.cc` have commented-out lines of code which could collect timing data, but since we have not yet needed that data, it is not saved to the file. This could be easily remedied. 


