import FWCore.ParameterSet.Config as cms

from RecoLocalFastTime.FTLCommonAlgos.mtdRecHitAlgo_cff import mtdRecHitAlgo

_barrelAlgo = mtdRecHitAlgo.clone()
_endcapAlgo = mtdRecHitAlgo.clone()

ftlRecHits = cms.EDProducer(
    "MTDRecHitProducer",
    barrel = _barrelAlgo,
    endcap = _endcapAlgo,
    barrelUncalibratedRecHits = cms.InputTag('ftlUncalibratedRecHits:FTLBarrel'),
    endcapUncalibratedRecHits = cms.InputTag('ftlUncalibratedRecHits:FTLEndcap'),
    BarrelHitsName = cms.string('FTLBarrel'),
    EndcapHitsName = cms.string('FTLEndcap')
)
