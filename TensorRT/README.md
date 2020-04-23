Remote mode:
```
cd python
cmsRun HcalTest_mc_cfg.py maxEvents=25 port=<port> address=<server_ip> batchsize=16000 modelname=facile maxEvents=10
```

All the different client options can be tested with an additional argument:
`mode=Async` (default), `mode=Sync`, `mode=PseudoAsync`.

Other available servers:
* `prp-gpu-1.t2.ucsd.edu`
