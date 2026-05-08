import FWCore.ParameterSet.Config as cms

hltLSTPixelHitsFromSoA = cms.EDProducer('LSTPixelHitsFromSoAProducer@alpaka',
    pixelRecHitsSoA = cms.InputTag('hltPhase2SiPixelRecHitsSoA'),
    mightGet = cms.optional.untracked.vstring,
    # autoselect the alpaka backend
    alpaka = cms.untracked.PSet(backend = cms.untracked.string(''))
)
