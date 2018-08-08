import sys
# Forked from SMPJ Analysis Framework
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW
# https://github.com/cms-smpj/SMPJ/tree/v1.0
# (further optimized to improve performance)


## Import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import *
import FWCore.Utilities.FileUtils as FileUtils

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# True : when running in OpenData virtual machine
# False: when runing in lxplus 
runOnVM = False

# Index of data files
#files2011data = FileUtils.loadListFromFile('CMS_Run2011A_Jet_AOD_12Oct2013-v1_20000_file_index.txt')
#process.source.fileNames = cms.untracked.vstring(*files2011data)
                                                             
process.source.fileNames = cms.untracked.vstring([sys.argv[2]])

# Read condition data from local sqlite database
if runOnVM:
    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db')


# Read good luminosity sections from local JSON file
import FWCore.PythonUtilities.LumiList as LumiList 
goodJSON = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions11/7TeV/Reprocessing/Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt'
myLumis = LumiList.LumiList(filename = goodJSON).getCMSSWString().split(',') 
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange() 
process.source.lumisToProcess.extend(myLumis)


# Global tag for 2011A data
process.GlobalTag.globaltag = 'FT_53_LV5_AN1::All'

# Select good vertices
process.goodOfflinePrimaryVertices = cms.EDFilter(
    "VertexSelector",
    filter = cms.bool(False),
    src = cms.InputTag("offlinePrimaryVertices"),
    cut = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.rho < 2")
    )

# Tracking failure filter
from RecoMET.METFilters.trackingFailureFilter_cfi import trackingFailureFilter
process.trackingFailureFilter = trackingFailureFilter.clone()
process.trackingFailureFilter.VertexSource = cms.InputTag('goodOfflinePrimaryVertices')

# Load jet correction services for all jet algoritms
process.load("JetMETCorrections.Configuration.JetCorrectionServicesAllAlgos_cff")

################### EDAnalyzer ##############################
process.ak5ak7 = cms.EDAnalyzer('OpenDataTreeProducerOptimized',
    ## numpy output                                                                                   
    maxRows = cms.untracked.int32(10000000),                            
    ## jet collections ###########################
    pfak7jets       = cms.InputTag('ak7PFJets'),
    pfak5jets       = cms.InputTag('ak5PFJets'),
    ## MET collection ####
    pfmet           = cms.InputTag('pfMET5'),
    ## set the conditions for good Vtx counting ##
    offlineVertices = cms.InputTag('goodOfflinePrimaryVertices'),
    goodVtxNdof     = cms.double(4), 
    goodVtxZ        = cms.double(24),
    ## rho #######################################
    srcPFRho        = cms.InputTag('kt6PFJets','rho'),
    ## preselection cuts #########################
    maxY            = cms.double(99.0), 
    maxEta          = cms.double(2.0), 
    minPFPt         = cms.double(100),
    minNPFJets      = cms.int32(1),
    minJJMass       = cms.double(-1),
    isMCarlo        = cms.untracked.bool(False),
    ## trigger ###################################
    printTriggerMenu = cms.untracked.bool(True),
    processName     = cms.string('HLT'),
    triggerNames    = cms.vstring(
                                    'HLT_Jet30', 'HLT_Jet60', 'HLT_Jet80', 'HLT_Jet110', 
                                    'HLT_Jet150', 'HLT_Jet190','HLT_Jet240','HLT_Jet370',
                                ),
    triggerResults  = cms.InputTag("TriggerResults","","HLT"),
    triggerEvent    = cms.InputTag("hltTriggerSummaryAOD","","HLT"),
    ## jet energy correction labels ##############
    jetCorr_ak5      = cms.string('ak5PFL1FastL2L3Residual'),
    jetCorr_ak7      = cms.string('ak7PFL1FastL2L3Residual'),
    # PF Candidates
    pfCandidates     = cms.InputTag("particleFlow","","RECO"),
)

# HLT filter
process.hltFilter = cms.EDFilter('HLTHighLevel',
    TriggerResultsTag  = cms.InputTag('TriggerResults','','HLT'),
    HLTPaths           = cms.vstring('HLT_Jet*', 'HLT_DiJetAve*'),
    eventSetupPathsKey = cms.string(''),
    andOr              = cms.bool(True), #----- True = OR, False = AND between the HLTPaths
    throw              = cms.bool(False)
)

# Run everything
process.p = cms.Path(
    process.goodOfflinePrimaryVertices*
    process.hltFilter *
    process.trackingFailureFilter *
    process.ak5ak7
)

# Approximate processing time on VM (Intel Core i5-5300U 2.3GHz laptop):
# 50000 events per 1 hour (both for DATA and MC)

# Change number of events here:
process.maxEvents.input = -1

process.MessageLogger.cerr.FwkReport.reportEvery = 500
process.MessageLogger.categories.append('OpenDataTreeProducerOptimized')
process.MessageLogger.cerr.OpenDataTreeProducerOptimized = cms.untracked.PSet(
    optionalPSet = cms.untracked.bool(True),
    limit = cms.untracked.int32(10000000),
)

# Output file
process.TFileService = cms.Service("TFileService", fileName = cms.string('OpenDataTree_data.root'))

# To suppress long output at the end of the job
#process.options.wantSummary = False   

del process.outpath
