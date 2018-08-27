from FWCore.ParameterSet.VarParsing import VarParsing
# Forked from SMPJ Analysis Framework
import FWCore.ParameterSet.Config as cms
import os
import sys

options = VarParsing("jet")
options.register("remote", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool)
options.register("address", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("port", -1, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("timeout", 30, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.parseArguments()

process = cms.Process('MakingBacon')

#--------------------------------------------------------------------------------
# Import of standard configurations
#================================================================================
process.load('FWCore/MessageService/MessageLogger_cfi')
#process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load('Configuration/StandardSequences/GeometryDB_cff')
process.load('Configuration/StandardSequences/MagneticField_38T_cff')

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = cms.string('100X_upgrade2018_realistic_v10')


# process.load('Configuration.StandardSequences.Services_cff')
# process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# True : when running in OpenData virtual machine
# False: when runing in lxplus 
# runOnVM = False

# Local input
#fileList = FileUtils.loadListFromFile(
    #'tmp.txt',
    #'CMS_MonteCarlo2011_Summer11LegDR_W1Jet_TuneZ2_7TeV-madgraph-tauola_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt',
    #'CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-80to120_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt'
#)

## https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset+dataset%3D%2FBulkGravTohhTohbbhbb_narrow_M-*_13TeV-madgraph%2FRunIISpring18MiniAOD-100X_upgrade2018_realistic_v10-v*%2FMINIAODSIM
# process.source.fileNames = cms.untracked.vstring([ "/store/mc/RunIISpring18MiniAOD/BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph/MINIAODSIM/100X_upgrade2018_realistic_v10-v1/30000/24A0230C-B530-E811-ADE3-14187741120B.root" ])

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )
process.source = cms.Source("PoolSource",
                            #fileNames = cms.untracked.vstring('/store/mc/RunIISummer16MiniAODv2/ZprimeToTT_M-4000_W-40_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/110000/02DEA6C9-19B7-E611-B22D-A0000420FE80.root'),
                            #fileNames = cms.untracked.vstring('/store/mc/RunIIFall17MiniAOD/QCD_HT1000to1500_TuneCP5_13TeV-madgraph-pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v1/50000/EEF28E00-0CEA-E711-8257-02163E0160F1.root'),
                            #fileNames = cms.untracked.vstring('/store/mc/RunIIFall17MiniAOD/QCD_HT2000toInf_TuneCP5_13TeV-madgraph-pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v1/20000/46FB5EDE-F708-E811-A50F-0025905C53A4.root')
                            fileNames = cms.untracked.vstring('file:store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root')
                            #skipEvents = cms.untracked.uint32(0),
)

# if runOnVM:
#     process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db')

# # Global tag for Summer11LegDR-PU_S13_START53_LV6-v1
# process.GlobalTag.globaltag = cms.string('START53_LV6A1::All')

# Select good vertices  -- don't need vertices -- they are in miniAOD
# process.goodOfflinePrimaryVertices = cms.EDFilter(
#     "VertexSelector",
#     filter = cms.bool(False),
#     src = cms.InputTag("offlinePrimaryVertices"),
#     cut = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.rho < 2")
#     )

# -------- The Tracking failure filter ------#
# no such thing as tracking failures!
# from RecoMET.METFilters.trackingFailureFilter_cfi import trackingFailureFilter
# process.trackingFailureFilter = trackingFailureFilter.clone()
# process.trackingFailureFilter.VertexSource = cms.InputTag('goodOfflinePrimaryVertices')

# Load jet correction services for all jet algoritms, we don't need this right?
# process.load("JetMETCorrections.Configuration.JetCorrectionServicesAllAlgos_cff")

################### EDProducer ##############################
process.jetImageProducer = cms.EDProducer('OpenDataTreeProducerOptimized',
    ## numpy output                                                                                                          
    maxRows = cms.untracked.int32(10000000),
    #JetTag          = cms.InputTag('slimmedJetsPuppi'),
    JetTag          = cms.InputTag('slimmedJetsAK8'),
    ## jet collections ###########################
    pfak7jets       = cms.InputTag('ak7PFJets'),
    pfak5jets       = cms.InputTag('ak5PFJets'),
    ## MET collection ####
    pfmet           = cms.InputTag('pfMET7'),
    ## database entry for the uncertainties ######
    PFPayloadName   = cms.string('AK7PF'),

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
    minGenPt        = cms.untracked.double(30),
    minJJMass       = cms.double(-1),
    isMCarlo        = cms.untracked.bool(True),
    genjets         = cms.untracked.InputTag('ak7GenJets'),
    genparticles    = cms.untracked.InputTag('genParticles'),
    useGenInfo      = cms.untracked.bool(True),
    ## trigger ###################################
    printTriggerMenu = cms.untracked.bool(True),
    processName     = cms.string('HLT'),
    triggerNames    = cms.vstring(
                                'HLT_Jet30', 'HLT_Jet60', 'HLT_Jet80', 'HLT_Jet110', 
                                'HLT_Jet150','HLT_Jet190','HLT_Jet240','HLT_Jet370',
                                ),
    triggerResults  = cms.InputTag("TriggerResults","","HLT"),
    triggerEvent    = cms.InputTag("hltTriggerSummaryAOD","","HLT"),
    ## jet energy correction labels ##############
    jetCorr_ak5      = cms.string('ak5PFL1FastL2L3Residual'),
    jetCorr_ak7      = cms.string('ak7PFL1FastL2L3Residual'),
    # PF Candidates
    pfCandidates     = cms.InputTag("particleFlow","","RECO"),
)

if options.remote:
    process.jetImageProducer.ServerParams = cms.PSet(
        address = cms.string(options.address),
        port = cms.int32(options.port),
        timeout = cms.uint32(options.timeout),
    )

############# hlt filter #########################
# process.hltFilter = cms.EDFilter('HLTHighLevel',
#     TriggerResultsTag  = cms.InputTag('TriggerResults','','HLT'),
#     HLTPaths           = cms.vstring('HLT_Jet*', 'HLT_DiJetAve*'),
#     eventSetupPathsKey = cms.string(''),
#     andOr              = cms.bool(True), #----- True = OR, False = AND between the HLTPaths
#     throw              = cms.bool(False)
# )


# Let it run
process.p = cms.Path(
    process.jetImageProducer
)


# Approximate processing time on VM (Intel Core i5-5300U 2.3GHz laptop):
# 50000 events per 1 hour (both for DATA and MC)

# Change number of events here:
#process.maxEvents.input = 1000
process.maxEvents.input = 25

process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.categories.append('OpenDataTreeProducerOptimized')
process.MessageLogger.cerr.OpenDataTreeProducerOptimized = cms.untracked.PSet(
    optionalPSet = cms.untracked.bool(True),
    limit = cms.untracked.int32(10000000),
)

# Output file
process.TFileService = cms.Service("TFileService", fileName = cms.string('OpenDataTree_mc.root'))

# To suppress long output at the end of the job
#process.options.wantSummary = False   

# del process.outpath
