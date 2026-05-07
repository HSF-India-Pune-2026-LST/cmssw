#include "LSTPixelHitsKernels.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  namespace {
    struct CopyHits {
      ALPAKA_FN_ACC void operator()(Acc1D const& acc,
                                    ::reco::TrackingRecHitConstView inHits,
                                    ::lst::LSTPixelHitsView outHits) const {
        auto nHits = inHits.metadata().size();
        for (unsigned int i : cms::alpakatools::uniform_elements(acc, nHits)) {
          outHits.ys()[i] = inHits.yGlobal()[i];
        }
      }
    };
  }
  
  void copyHits(Queue& queue, reco::TrackingRecHitsSoACollection const& input, lst::LSTPixelHitsDeviceCollection& output) {
    auto const workDiv = cms::alpakatools::make_workdiv<Acc1D>(1, input.nHits());
    alpaka::exec<Acc1D>(queue, workDiv, CopyHits{}, input.const_view().trackingHits(), output.view());
    
  }

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE
