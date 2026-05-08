#ifndef RecoTracker_LST_plugins_alpaka_LSTPixelHitsKernels_h
#define RecoTracker_LST_plugins_alpaka_LSTPixelHitsKernels_h

#include "DataFormats/TrackingRecHitSoA/interface/TrackingRecHitsDevice.h"
#include "RecoTracker/LSTCore/interface/alpaka/LSTPixelHitsDeviceCollection.h"
#include "DataFormats/TrackingRecHitSoA/interface/alpaka/TrackingRecHitsSoACollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  namespace lstpixel {
  void copyHits(Queue& queue, ::reco::TrackingRecHitConstView input, ::lst::LSTPixelHitsView output);
  }
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

#endif
