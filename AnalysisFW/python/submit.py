import os
samples = ['CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-120to170_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index',
           'CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-170to300_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index',
           'CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-300to470_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index',
           'CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-470to600_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index',
           'CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-80to120_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index',
           'CMS_MonteCarlo2011_Summer11LegDR_TT_weights_CT10_TuneZ2_7TeV-powheg-pythia-tauola_AODSIM_PU_S13_START53_LV6-v2_00000_file_index'
           ]


if __name__ == '__main__':
    for sample in samples:        
        #os.system('python baconBatch.py cmsRun params0.npy -a 1:OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py  --list 2:%s.txt --outdir %s --eosoutdir root://cmseos.fnal.gov//eos/uscms/store/user/woodson/DSHEP2017/%s -q 1nh -n 8000'%(sample,sample,sample))
        os.system('python baconBatch.py cmsRun params0.npy -a 1:OpenDataTreeProducerOptimized_mcPAT_2011_cfg.py  --list 2:%s.txt --outdir %s --eosoutdir root://cmseos.fnal.gov//eos/uscms/store/user/woodson/DSHEP2017/%s -q 1nh -n 8000 --monitor resub'%(sample,sample,sample))
