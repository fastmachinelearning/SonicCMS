from FWCore.ParameterSet.VarParsing import VarParsing
# Forked from SMPJ Analysis Framework
import FWCore.ParameterSet.Config as cms
import os, sys, json

options = VarParsing("analysis")
options.register("remote", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool)
options.register("address", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("port", -1, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("timeout", 30, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("params", "", VarParsing.multiplicity.singleton, VarParsing.varType.string)
options.register("threads", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.register("streams", 0, VarParsing.multiplicity.singleton, VarParsing.varType.int)
options.parseArguments()

if len(options.params)>0 and options.remote:
    with open(options.params,'r') as pfile:
        pdict = json.load(pfile)
    options.address = pdict["address"]
    options.port = int(pdict["port"])
    print("server = "+options.address+":"+str(options.port))

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
    fileNames = cms.untracked.vstring('file:data/store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root')
)

if len(options.inputFiles)>0: process.source.fileNames = options.inputFiles

################### EDProducer ##############################
process.jetImageProducer = cms.EDProducer('JetImageProducer',
    JetTag = cms.InputTag('slimmedJetsAK8'),
    topN = cms.uint32(5),
    imageList = cms.string("imagenet_classes.txt"),
)

if options.remote:
    process.jetImageProducer.remote = cms.bool(True)
    process.jetImageProducer.ExtraParams = cms.PSet(
        address = cms.string(options.address),
        port = cms.int32(options.port),
        timeout = cms.uint32(options.timeout),
    )
else:
    process.jetImageProducer.remote = cms.bool(False)
    process.jetImageProducer.ExtraParams = cms.PSet(
        featurizer = cms.string("resnet50.pb"),
        classifier = cms.string("resnet50_classifier.pb"),
    )

# Let it run
process.p = cms.Path(
    process.jetImageProducer
)

process.MessageLogger.cerr.FwkReport.reportEvery = 1
keep_msgs = ['JetImageProducer','TFClientRemote','TFClientLocal']
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
