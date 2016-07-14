
from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.General.requestName = 'ProcessedTree_test'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'ProcessedTreeProducer_dataPAT_2011_cfg.py'
config.JobType.inputFiles = ['Legacy11_V1_DATA_UncertaintySources_AK7PFchs.txt',
                            'Legacy11_V1_DATA_UncertaintySources_AK7PF.txt',
                            'Legacy11_V1_DATA_UncertaintySources_AK5PFchs.txt',
                            'Legacy11_V1_DATA_UncertaintySources_AK5PF.txt']

config.Data.inputDataset = '/Jet/Run2011A-12Oct2013-v1/AOD'
config.Data.inputDBS = 'global'
config.Data.splitting = 'LumiBased'
config.Data.unitsPerJob = 2
config.Data.lumiMask = './Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt'
config.Data.runRange = '160431-160431'
#config.Data.outLFNDirBase = '/store/group/phys_smp/mhaapale/'
config.Data.outputDatasetTag = 'ProcessedTree_test'

config.Site.storageSite = 'T2_FI_HIP'