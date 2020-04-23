from FWCore.ParameterSet.VarParsing import VarParsing
import FWCore.ParameterSet.Config as cms
import os, sys, json
from datetime import datetime

options = VarParsing("analysis")
options.register("address", "34.66.125.42", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("inputfile", "root://cmsxrootd.fnal.gov//store/relval/CMSSW_10_6_0/RelValTTbar_13/GEN-SIM-DIGI-RAW/106X_upgrade2021_realistic_v5_LowSigmaZGTv5-v1/10000/42E44201-9A4C-C74E-838B-2215221081BC.root", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("port", 8001, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("timeout", 300, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("params", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("threads", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("streams", 0,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("batchsize", 16000,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("modelname","facile_all_v2", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("mode", "Async", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("hang", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.parseArguments()

# reset args to avoid collision w/ customizeHLTforAll
sys.argv = sys.argv[0:1]

if len(options.params)>0:
    with open(options.params,'r') as pfile:
        pdict = json.load(pfile)
    options.address = pdict["address"]
    options.port = int(pdict["port"])
    print("server = "+options.address+":"+str(options.port))

# check mode
allowed_modes = {
      
    "Async": "HcalPhase1Reconstructor_FACILEAsync",
    "Sync": "HcalPhase1Reconstructor_FACILESync",
    "PseudoAsync": "HcalPhase1Reconstructor_FACILEPseudoAsync",
}

from Configuration.StandardSequences.Eras import eras
#process = cms.Process('HLT',eras.Run3)
#process.load('HLTrigger.Configuration.HLT_GRun_cff')


sys.path.insert(0,os.path.expandvars("$CMSSW_RELEASE_BASE/src/HLTrigger/Configuration/test"))
from OnLine_HLT_GRun import process

process.hltHbhereco = cms.EDProducer(allowed_modes[options.mode],
    sipmQTSShift = cms.uint32(0),
    sipmQNTStoSum = cms.uint32(3),
    topN = cms.uint32(5),
    edmRecHitName = cms.InputTag("hbheprereco"),
    edmChanInfoName = cms.InputTag("hbheprereco"),
    digiLabelQIE11 = cms.InputTag("simHcalDigis","HBHEQIE11DigiCollection"),
    simHcalDigiName = cms.untracked.InputTag("simHcalDigis","HBHEQIE11DigiCollection"),
    Client = cms.PSet(
        ninput  = cms.uint32(47),
        noutput = cms.uint32(1),
        batchSize = cms.uint32(options.batchsize),
        address = cms.string(options.address),
        port = cms.uint32(options.port),
        timeout = cms.uint32(options.timeout),
        modelName = cms.string(options.modelname)
    )
)


# add specific customizations
_customInfo = {}
_customInfo['menuType'  ]= "GRun"
_customInfo['globalTags']= {}
_customInfo['globalTags'][True ] = "106X_upgrade2021_realistic_v5" #"auto:phase1_2021_realistic"#106X_mc2017_realistic_v3"
_customInfo['globalTags'][False] = "106X_upgrade2021_realistic_v5" #"auto:phase1_2021_realistic"#106X_mc2017_realistic_v3"
_customInfo['inputFiles']={}
_customInfo['inputFiles'][True]  = "file:default.root"
_customInfo['inputFiles'][False]= [options.inputfile]
_customInfo['maxEvents' ]=  10
_customInfo['reportEvery' ]=  10
_customInfo['globalTag' ]= "106X_upgrade2021_realistic_v5" #"auto:phase1_2021_realistic"#106X_mc2017_realistic_v3"
_customInfo['inputFile' ]= [options.inputfile]
_customInfo['realData'  ]=  False
from HLTrigger.Configuration.customizeHLTforALL import customizeHLTforAll
process = customizeHLTforAll(process,"GRun",_customInfo)

from HLTrigger.Configuration.customizeHLTforCMSSW import customizeHLTforCMSSW
process = customizeHLTforCMSSW(process,"GRun")

process.load('FWCore/MessageService/MessageLogger_cfi')
keep_msgs = ['TRTClient']
for msg in keep_msgs:
    process.MessageLogger.categories.append(msg)
    setattr(process.MessageLogger.cerr,msg,
        cms.untracked.PSet(
            optionalPSet = cms.untracked.bool(True),
            limit = cms.untracked.int32(10000000),
        )
    )

if options.threads>0:
    if not hasattr(process,"options"):
        process.options = cms.untracked.PSet()
    process.options.numberOfThreads = cms.untracked.uint32(options.threads)
    process.options.numberOfStreams = cms.untracked.uint32(options.streams if options.streams>0 else 0)

#from HLTrigger.Configuration.customizeHLTforMC import customizeHLTforMC 
#process = customizeHLTforMC(process)

# instrument the menu with the FastTimerService
process.load( "HLTrigger.Timer.FastTimerService_cfi" )

# print a text summary at the end of the job
process.FastTimerService.printEventSummary        = False
process.FastTimerService.printRunSummary          = False
process.FastTimerService.printJobSummary          = True

