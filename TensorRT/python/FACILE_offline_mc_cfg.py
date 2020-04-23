from FWCore.ParameterSet.VarParsing import VarParsing
import FWCore.ParameterSet.Config as cms
import os, sys, json
from datetime import datetime

options = VarParsing("analysis")
options.register("address", "ailab01.fnal.gov", VarParsing.multiplicity.singleton, VarParsing.varType.string)
#options.register("address", "prp-gpu-1.t2.ucsd.edu", VarParsing.multiplicity.singleton, VarParsing.varType.string)
#options.register("inputfile", "step2.root", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("inputfile", "root://cmsxrootd.fnal.gov//store/relval/CMSSW_10_6_0/RelValTTbar_13/GEN-SIM-DIGI-RAW/106X_upgrade2021_realistic_v5_LowSigmaZGTv5-v1/10000/42E44201-9A4C-C74E-838B-2215221081BC.root", VarParsing.multiplicity.singleton, VarParsing.varType.string)
#options.register("address", "18.4.112.82", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("port", 8001, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("timeout", 300, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("params", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("threads", 4, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("streams", 0,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("batchsize", 16000,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("modelname","facile_all_v2", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("mode", "Async", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("hang", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.parseArguments()

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
if options.mode not in allowed_modes:
    raise ValueError("Unknown mode: "+options.mode)

from Configuration.StandardSequences.Eras import eras
process = cms.Process('HcalTest',eras.Run3)

#--------------------------------------------------------------------------------
# Import of standard configurations
#================================================================================
process.load('FWCore/MessageService/MessageLogger_cfi')
process.load('Configuration/StandardSequences/GeometryDB_cff')
process.load('Configuration/StandardSequences/MagneticField_38T_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('HLTrigger.Configuration.HLT_GRun_cff')
#process.GlobalTag.globaltag = cms.string('auto:phase1_2021_realistic')#100X_upgrade2018_realistic_v10')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')


process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.source = cms.Source("PoolSource",
    #fileNames = cms.untracked.vstring('file:../../Core/data/store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root')
    fileNames = cms.untracked.vstring(['file:'+options.inputfile]*100),
    duplicateCheckMode = cms.untracked.string('noDuplicateCheck')
)

if len(options.inputFiles)>0: process.source.fileNames = options.inputFiles

###################### Hang #################################

if (options.hang != ""):
    l = options.hang.split(":")
    if not (len(l) == 2):
         raise Exception("Hang improperly formatted")
    hour = int(l[0])
    minute = int(l[1])
    if not (0 <= hour < 24):
        raise Exception("Hour not in proper range")
    if not (0 <= minute < 60):
        raise Exception("Minute not in proper range")
    print("Waiting until " + options.hang+".")
    hang = True

    while hang:
        nowHour = datetime.now().hour
        nowMinute = datetime.now().minute
        hang = not (hour <= nowHour and minute <= nowMinute)
    print("Signal received")

################### EDProducer ##############################
process.HcalProducer = cms.EDProducer(allowed_modes[options.mode],
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

process.HcalProducer_step = cms.Path(process.HcalProducer) 
process.endjob_step = cms.EndPath(process.endOfProcess)

process.schedule = cms.Schedule(process.HcalProducer_step,process.endjob_step)

process.MessageLogger.cerr.FwkReport.reportEvery = 1
keep_msgs = ['TRTClient','HcalProducer']
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

from HLTrigger.Configuration.customizeHLTforMC import customizeHLTforMC 
process = customizeHLTforMC(process)


# instrument the menu with the FastTimerService
process.load( "HLTrigger.Timer.FastTimerService_cfi" )

# print a text summary at the end of the job
process.FastTimerService.printEventSummary        = False
process.FastTimerService.printRunSummary          = False
process.FastTimerService.printJobSummary          = True

