import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing

process = cms.Process("USER")

process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
process.load("FWCore.MessageLogger.MessageLogger_cfi")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag (process.GlobalTag, 'auto:run2_mc')

## Events to process
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

## Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'root://gfe02.grid.hep.ph.ic.ac.uk/pnfs/hep.ph.ic.ac.uk/data/cms/store/user/mkomm/LLP/LLP_miniaodv180920/SMS-T1qqqq_ctau-10_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/SMS-T1qqqq_ctau-10_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/LLP_miniaodv180920/180926_191050/0000/T1qqqqLL_MINIAODSIM_82.root'
    )
)

## Output file
from PhysicsTools.PatAlgos.patEventContent_cff import patEventContent
process.OUT = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('test.root'),
    outputCommands = cms.untracked.vstring(['keep *'])
)

from PhysicsTools.PatAlgos.tools.jetTools import *

updateJetCollection(
    process,
    jetSource=cms.InputTag("slimmedJets"),
    jetCorrections=("AK4PFchs", cms.vstring(["L1FastJet", "L2Relative", "L3Absolute"]), "None"),
    btagInfos = ['pfImpactParameterTagInfos', 'pfInclusiveSecondaryVertexFinderTagInfos', 'pfDeepCSVTagInfos']
)

process.updatedPatJets.addBTagInfo = cms.bool(True)
process.updatedPatJets.addDiscriminators = cms.bool(True)
process.updatedPatJets.addJetCorrFactors = cms.bool(True)
process.updatedPatJets.addTagInfos = cms.bool(True)


process.pfXTagInfos = cms.EDProducer("XTagInfoProducer",
    jets = cms.InputTag("updatedPatJets"),
    shallow_tag_infos = cms.InputTag('pfDeepCSVTagInfos'),
    vertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
    secondary_vertices = cms.InputTag("slimmedSecondaryVertices")
)

process.pfXTags = cms.EDProducer("XTagProducer",
    graph_path=cms.FileInPath("LLPReco/TensorFlow/data/da.pb"),
    src=cms.InputTag("pfXTagInfos")
)

process.options = cms.untracked.PSet(
        wantSummary = cms.untracked.bool(True)
)

process.task = cms.Task(
        process.pfImpactParameterTagInfos,
        process.pfInclusiveSecondaryVertexFinderTagInfos,
        process.pfDeepCSVTagInfos,
        process.patJetCorrFactors,
        process.pfXTagInfos,
        process.pfXTags,
        process.updatedPatJets
)
process.p = cms.Path(process.task)

process.endpath= cms.EndPath(process.OUT)
