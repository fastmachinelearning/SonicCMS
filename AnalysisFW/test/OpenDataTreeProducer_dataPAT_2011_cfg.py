## import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import *
import FWCore.Utilities.FileUtils as FileUtils

runOnVM = False


# Local input

files2011data = FileUtils.loadListFromFile('CMS_Run2011A_Jet_AOD_12Oct2013-v1_20000_file_index.txt')
process.source.fileNames = cms.untracked.vstring(*files2011data)

# Grid: files in CRAB config
#process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')


# ONLY in VM!!
if runOnVM:
    process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db')

# Select good luminosity section using a local JSON file
import FWCore.PythonUtilities.LumiList as LumiList 
goodJSON = './Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt' 
myLumis = LumiList.LumiList(filename = goodJSON).getCMSSWString().split(',') 
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange() 
process.source.lumisToProcess.extend(myLumis)


process.GlobalTag.globaltag = 'FT_53_LV5_AN1::All'


# load the PAT config
process.load("RecoTauTag.Configuration.RecoPFTauTag_cff") # re-run tau discriminators (new version)
process.load("PhysicsTools.PatAlgos.patSequences_cff")
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('RecoJets.Configuration.RecoPFJets_cff')
process.load('RecoJets.Configuration.RecoJets_cff')
process.load('RecoJets.JetProducers.TrackJetParameters_cfi')
process.load('JetMETCorrections.Configuration.DefaultJEC_cff')


# Configure PAT to use PF2PAT instead of AOD sources
# this function will modify the PAT sequences.
from PhysicsTools.PatAlgos.tools.pfTools import *
from PhysicsTools.PatAlgos.tools.coreTools import *
from PhysicsTools.PatAlgos.tools.metTools import *
from PhysicsTools.PatAlgos.tools.jetTools import *
from PhysicsTools.PatAlgos.tools.coreTools import *
from PhysicsTools.SelectorUtils.pvSelector_cfi import pvSelector



process.goodOfflinePrimaryVertices = cms.EDFilter(
    "VertexSelector",
    filter = cms.bool(False),
    src = cms.InputTag("offlinePrimaryVertices"),
    cut = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.rho < 2")
    )


process.ak7PFJets.doAreaFastjet = True
process.kt6PFJets.doRhoFastjet = True


# Configure PAT to use PF2PAT instead of AOD sources
# this function will modify the PAT sequences.
from PhysicsTools.PatAlgos.tools.pfTools import *

#------------- Running a second PF2PAT for Ak7chs jets ----------------#
postfix2 = "CHS7"
jetAlgo2 = "AK7"
usePF2PAT(process,runPF2PAT=True, jetAlgo=jetAlgo2, runOnMC=False, postfix=postfix2,
         jetCorrections=('AK7PFchs', ['L1FastJet','L2Relative','L3Absolute','L2L3Residual']),
         pvCollection=cms.InputTag('goodOfflinePrimaryVertices'),
         typeIMetCorrections=True
         )


# Does this need to be modified??
# --- modifying the pfpileup+postfix2 -------------------- #
process.pfPileUpCHS7.checkClosestZVertex = False
process.pfPileUpCHS7.Enable = True
process.pfPileUpCHS7.Vertices = cms.InputTag('goodOfflinePrimaryVertices')
process.pfJetsCHS7.doAreaFastjet = True
process.pfJetsCHS7.doRhoFastjet = False

process.ak5PFJets.doAreaFastjet = True
process.ak7PFJets.doAreaFastjet = True
process.kt6PFJets.doRhoFastjet = True

# What about this??
# to use tau-cleaned jet collection uncomment the following:
getattr(process,"pfNoTau"+postfix2).enable = True


# Leave or remove??
#------ removing the MC matching and let it run -------#
removeMCMatchingPF2PAT( process, '' )
runOnData(process)

# ------------- Adding Ak7 jet collection to process ------------- #
addPfMET(process, 'PF')

# ------- Adding non CHS jets to process ------//
addJetCollection(process,cms.InputTag('ak7PFJets'),
                 'AK7', 'PFCorr',
                 doJTA        = True,
                 doBTagging   = False,
                 jetCorrLabel = ('AK7PF', cms.vstring(['L1FastJet','L2Relative','L3Absolute','L2L3Residual'])),
                 doType1MET   = True,
                 doL1Cleaning = True,
                 doL1Counters = False,
#                 genJetCollection=cms.InputTag("ak7GenJets"),
                 doJetID      = True,
                 jetIdLabel   = "ak7"
                 )


# -------- The Tracking failure filter ------#
from RecoMET.METFilters.trackingFailureFilter_cfi import trackingFailureFilter
process.trackingFailureFilter = trackingFailureFilter.clone()
process.trackingFailureFilter.VertexSource = cms.InputTag('goodOfflinePrimaryVertices')



################### declaring the EDAnalyzer ##############################3

process.ak7 = cms.EDAnalyzer('OpenDataTreeProducer',
    ## jet collections ###########################
    pfjets          = cms.InputTag('selectedPatJetsAK7PFCorr'),
    ## MET collection ####
    pfmet           = cms.InputTag('pfMETCHS7'),
    ## database entry for the uncertainties ######
    PFPayloadName   = cms.string('AK7PF'),
    #PFPayloadNameCHS= cms.string('AK7PFchs'),

    ## set the conditions for good Vtx counting ##
    offlineVertices = cms.InputTag('goodOfflinePrimaryVertices'),
    goodVtxNdof     = cms.double(4), 
    goodVtxZ        = cms.double(24),
    ## rho #######################################
    srcPFRho        = cms.InputTag('kt6PFJets','rho'),
    ## preselection cuts #########################
    maxY            = cms.double(5.0), 
    minPFPt         = cms.double(20),
    #minPFFatPt      = cms.double(10),
    #maxPFFatEta     = cms.double(2.5),
    minNPFJets      = cms.int32(1),
    minJJMass       = cms.double(-1),
    isMCarlo        = cms.untracked.bool(False),
    ## trigger ###################################
    printTriggerMenu = cms.untracked.bool(True),
    processName     = cms.string('HLT'),
    triggerNames    = cms.vstring(
                                'HLT_Jet30', 'HLT_Jet60', 'HLT_Jet80', 'HLT_Jet110', 'HLT_Jet150', 
                                'HLT_Jet190','HLT_Jet240','HLT_Jet370',
                                ),
    triggerResults  = cms.InputTag("TriggerResults","","HLT"),
    triggerEvent    = cms.InputTag("hltTriggerSummaryAOD","","HLT"),
    pfjecService    = cms.string('ak7PFL1FastL2L3Residual')
)


############# hlt filter #########################
process.hltFilter = cms.EDFilter('HLTHighLevel',
    TriggerResultsTag  = cms.InputTag('TriggerResults','','HLT'),
    HLTPaths           = cms.vstring('HLT_Jet*', 'HLT_DiJetAve*'),
    eventSetupPathsKey = cms.string(''),
    andOr              = cms.bool(True), #----- True = OR, False = AND between the HLTPaths
    throw              = cms.bool(False)
)


# Let it run
process.p = cms.Path(
    process.goodOfflinePrimaryVertices*
    process.hltFilter *
    process.trackingFailureFilter *
    process.patDefaultSequence *
    getattr(process,"patPF2PATSequence"+postfix2) *
    process.ak7
)

# Processing time on VM (2011 laptop)
# DATA: 50000 events / 4 hours
# MC:   50000 events / 5 hours
process.maxEvents.input = 100
process.MessageLogger.cerr.FwkReport.reportEvery = 5

process.TFileService = cms.Service("TFileService",fileName = cms.string('OpenDataTree_data.root'))

process.options.wantSummary = False   ##  (to suppress the long output at the end of the job)

del process.outpath
