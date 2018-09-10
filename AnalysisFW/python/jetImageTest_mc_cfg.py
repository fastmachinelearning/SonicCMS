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

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )
process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring('file:store_mc_RunIISpring18MiniAOD_BulkGravTohhTohbbhbb_narrow_M-2000_13TeV-madgraph_MINIAODSIM_100X_upgrade2018_realistic_v10-v1_30000_24A0230C-B530-E811-ADE3-14187741120B.root')
)

################### EDProducer ##############################
process.jetImageProducer = cms.EDProducer('JetImageProducer',
    JetTag          = cms.InputTag('slimmedJetsAK8'),
)

if options.remote:
    process.jetImageProducer.ServerParams = cms.PSet(
        address = cms.string(options.address),
        port = cms.int32(options.port),
        timeout = cms.uint32(options.timeout),
    )

# Let it run
process.p = cms.Path(
    process.jetImageProducer
)

# Change number of events here:
process.maxEvents.input = 25

process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.categories.append('JetImageProducer')
process.MessageLogger.cerr.JetImageProducer = cms.untracked.PSet(
    optionalPSet = cms.untracked.bool(True),
    limit = cms.untracked.int32(10000000),
)
process.MessageLogger.categories.append('TFClient')
process.MessageLogger.cerr.TFClient = cms.untracked.PSet(
    optionalPSet = cms.untracked.bool(True),
    limit = cms.untracked.int32(10000000),
)
