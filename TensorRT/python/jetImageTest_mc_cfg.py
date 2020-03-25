from FWCore.ParameterSet.VarParsing import VarParsing
# Forked from SMPJ Analysis Framework
import FWCore.ParameterSet.Config as cms
import os, sys, json

options = VarParsing("analysis")
options.register("address", "prp-gpu-1.t2.ucsd.edu", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("port", 8001, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("timeout", 30, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("params", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("threads", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("streams", 0,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("batchsize", 10,    VarParsing.multiplicity.singleton, VarParsing.varType.int)
#options.register("modelname","resnet50_netdef", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("modelname","resnet50_ensemble", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("mode","Async", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.parseArguments()

if len(options.params)>0:
    with open(options.params,'r') as pfile:
        pdict = json.load(pfile)
    options.address = pdict["address"]
    options.port = int(pdict["port"])
    print("server = "+options.address+":"+str(options.port))

# check mode
allowed_modes = {
    "Async": "JetImageProducerAsync",
    "Sync": "JetImageProducerSync",
    "PseudoAsync": "JetImageProducerPseudoAsync",
}
if options.mode not in allowed_modes:
    raise ValueError("Unknown mode: "+options.mode)

process = cms.Process('imageTest')

#--------------------------------------------------------------------------------
# Import of standard configurations
#================================================================================
process.load('FWCore/MessageService/MessageLogger_cfi')
process.load('Configuration/StandardSequences/GeometryDB_cff')
process.load('Configuration/StandardSequences/MagneticField_38T_cff')

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = cms.string('100X_upgrade2018_realistic_v10')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:../../Core/data/store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root')
)

if len(options.inputFiles)>0: process.source.fileNames = options.inputFiles

################### EDProducer ##############################
process.jetImageProducer = cms.EDProducer(allowed_modes[options.mode],
    JetTag = cms.InputTag('slimmedJetsAK8'),
    topN = cms.uint32(5),
    imageList = cms.string("../../Core/data/imagenet_classes.txt"),
    Client = cms.PSet(
        ninput  = cms.uint32(15),
        noutput = cms.uint32(1),
        batchSize = cms.uint32(options.batchsize),
        address = cms.string(options.address),
        port = cms.uint32(options.port),
        timeout = cms.uint32(options.timeout),
        modelName = cms.string(options.modelname),
    )
)

# Let it run
process.p = cms.Path(
    process.jetImageProducer
)

process.MessageLogger.cerr.FwkReport.reportEvery = 1
keep_msgs = ['JetImageProducer','TRTClient']
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
