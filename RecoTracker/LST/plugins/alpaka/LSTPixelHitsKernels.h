#include "DataFormats/TrackingRecHitSoA/interface/TrackingRecHitsDevice.h"
#include "RecoTracker/LSTCore/interface/alpaka/LSTPixelHitsDeviceCollection.h"
#include "DataFormats/TrackingRecHitSoA/interface/alpaka/TrackingRecHitsSoACollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  void copyHits(Queue& queue, ::reco::TrackingRecHitConstView input, ::lst::LSTPixelHitsView output);

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE
