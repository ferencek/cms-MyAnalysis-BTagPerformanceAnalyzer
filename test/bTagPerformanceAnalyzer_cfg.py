import FWCore.ParameterSet.Config as cms

process = cms.Process("USER")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.MessageLogger.cerr.default.limit = 10

process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.load("RecoBTag.PerformanceDB.PoolBTagPerformanceDB1107")
process.load("RecoBTag.PerformanceDB.BTagPerformanceDB1107")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.source = cms.Source("EmptySource")

# Output ROOT file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('btag_scale_factors.root')
)

process.bTagPerformanceAnalyzer = cms.EDAnalyzer('BTagPerformanceAnalyzer',
    BTagAlgoWP = cms.vstring("TCHEM", "TCHPT", "SSVHEM", "SSVHPT"),
    PtNBins    = cms.int32(50),
    PtMin      = cms.double(20.),
    PtMax      = cms.double(520),
    etaNBins   = cms.int32(30),
    etaMin     = cms.double(0.),
    etaMax     = cms.double(3.)
)

process.p = cms.Path(process.bTagPerformanceAnalyzer)
