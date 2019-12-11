Remote mode:
```
cd python
cmsRun jetImageTest_mc_cfg.py address=ailab01.fnal.gov port=8001 maxEvents=25
```
(or the same for `HcalTest_mc_cfg.py`)

All the different client options can be tested with an additional argument:
`mode=Async` (default), `mode=Sync`, `mode=PseudoAsync`.

Other available servers:
* `prp-gpu-1.t2.ucsd.edu`
