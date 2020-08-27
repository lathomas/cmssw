import FWCore.ParameterSet.Config as cms

HFJetShowerShape = cms.EDProducer('HFJetShowerShape',
  TheJets = cms.InputTag('slimmedJets'),
  TheVertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
  JetPtThreshold = cms.double(25),
  JetEtaThreshold = cms.double(2.9),
  HFTowerEtaWidth = cms.double(0.175),
  HFTowerPhiWidth = cms.double(0.175),
  VertexRecoEffcy = cms.double(0.7),
  OffsetPerPU = cms.double(0.4),
  JetReferenceRadius = cms.double(0.4),
  StripPtThreshold = cms.double(10),
  WidthPtThreshold = cms.double(3)
)

HFJetShowerShapeTask = cms.Task(HFJetShowerShape)
