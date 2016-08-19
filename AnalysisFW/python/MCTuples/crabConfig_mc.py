
'''

Python script to submit CRAB3 jobs processing several Monte Carlo samples

Run the following commands from the parent directory: 
(you might want to double-check paths and copy scripts to the correct locations)

1. Submit CRAB3 jobs
    cmsenv && source /cvmfs/cms.cern.ch/crab3/crab.sh && python MCTuples/crabConfig_mc.py
If the jobs crash (error 8002), try checking that in the CMSSW config 'runOnVM' is False!

2. Merge the tuples of each sample: get one file per pthat interval
    bsub -q 1nh -J job1 < MCTuples/mergeMCTuples.sh
3. Create the final Monte Carlo tuple file
(the ROOT script does the merging, while also computing: weight = cross-section / num of events):
    bsub -q 1nh -J job1 < MCTuples/createMCTuple.sh 

'''


from CRABClient.UserUtilities import config, getUsernameFromSiteDB

config = config()

# Global CRAB3 settings
config.General.requestName = 'OpenDataTree_all'
config.General.workArea = 'crab_projects_retry'
config.General.transferOutputs = True
config.General.transferLogs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'OpenDataTreeProducer_mcPAT_2011_cfg.py'

config.Data.inputDBS = 'global'
config.Data.splitting = 'EventAwareLumiBased' #'FileBased'
config.Data.outLFNDirBase = '/store/group/phys_smp/mhaapale/'
config.Data.publication = False

config.Data.totalUnits = 1000000    # appr. number of events
config.Data.unitsPerJob = 40000     # appr. events per job

config.Site.storageSite = 'T2_CH_CERN'


# The following chunk was taken from:
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRABClientLibraryAPI?rev=29
if __name__ == '__main__':

    from CRABAPI.RawCommand import crabCommand
    from CRABClient.ClientExceptions import ClientException
    from httplib import HTTPException
    from multiprocessing import Process

    # Helper functions
    def submit(config):
        try:
            crabCommand('submit', config = config)
            #crabCommand('submit', config = config, dryrun = True) # Debug purposes
        except HTTPException as hte:
            print "Failed submitting task: %s" % (hte.headers)
        except ClientException as cle:
            print "Failed submitting task: %s" % (cle)

    def extractName(dataset):
        # Return shorter dataset name, e.g. "pthat_15to30"
        import re
        m = re.search('QCD_Pt-(\d+to\d+)_TuneZ2_7TeV_pythia6', dataset.split('/')[1])
        return "pthat_" + m.group(1)


    # Loop over MC datasets
    for dataset in [#'/QCD_Pt-0to5_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    #,'/QCD_Pt-5to15_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                     '/QCD_Pt-15to30_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-30to50_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-50to80_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-80to120_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-120to170_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-170to300_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-300to470_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-470to600_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-600to800_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ,'/QCD_Pt-800to1000_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    #,'/QCD_Pt-1000to1400_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    #,'/QCD_Pt-1400to1800_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    #,'/QCD_Pt-1800_TuneZ2_7TeV_pythia6/Summer11LegDR-PU_S13_START53_LV6-v1/AODSIM'
                    ]:

        config.Data.inputDataset = dataset
        config.General.requestName = extractName(dataset)


        # We use a multiprocessing trick explained here:
        # https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3FAQ?rev=56#Multiple_submission_fails_with_a
        # Reason: calling repeatedly 'submit(config)' gives an error (possibly related to the PAT modules)
        p = Process(target=submit, args=(config,))
        p.start()
        p.join()

