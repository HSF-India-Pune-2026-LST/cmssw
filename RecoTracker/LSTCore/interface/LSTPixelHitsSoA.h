#ifndef RecoTracker_LSTCore_interface_LSTPixelHitsSoA_h
#define RecoTracker_LSTCore_interface_LSTPixelHitsSoA_h

#ifndef LST_STANDALONE
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#endif

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoABlocks.h"
#include "DataFormats/Portable/interface/PortableCollection.h"

#include "RecoTracker/LSTCore/interface/Common.h"

namespace lst {

  GENERATE_SOA_LAYOUT(LSTPixelHitsSoALayout,
                      SOA_COLUMN(float, xs),
                      SOA_COLUMN(float, ys),
                      SOA_COLUMN(float, zs),
                      SOA_COLUMN(unsigned int, idxs),
                      SOA_COLUMN(unsigned int, detid),
                      SOA_COLUMN(uint16_t, clustsizeX),
                      SOA_COLUMN(uint16_t, clustsizeY)
#ifndef LST_STANDALONE
                          ,
                      SOA_COLUMN(TrackingRecHit const*, hits)
#endif
  )

  using LSTPixelHitsSoA = LSTPixelHitsSoALayout<>;

  using LSTPixelHitsView = LSTPixelHitsSoA::View;
  using LSTPixelHitsConstView = LSTPixelHitsSoA::ConstView;

}  // namespace lst

#endif
