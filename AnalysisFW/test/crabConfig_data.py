
from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.General.requestName = 'OpenDataTree_all'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.JobType.pluginName = 'Analysis'
config.JobType.maxJobRuntimeMin = 2000
config.JobType.psetName = 'OpenDataTreeProducer_dataPAT_2011_cfg.py'

config.Data.inputDataset = '/Jet/Run2011A-12Oct2013-v1/AOD'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 3
config.Data.lumiMask = './Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt'
config.Data.runRange = '160404-173692'
config.Data.outLFNDirBase = '/store/group/phys_smp/mhaapale/'
config.Data.publication = False

config.Site.storageSite = 'T2_CH_CERN' # or 'T2_FI_HIP'
