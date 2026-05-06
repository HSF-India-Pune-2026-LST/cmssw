#ifndef RecoTracker_LSTCore_interface_LSTPixelHitsHostCollection_h
#define RecoTracker_LSTCore_interface_LSTPixelHitsHostCollection_h

#include "RecoTracker/LSTCore/interface/LSTPixelHitsSoA.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"
#include "DataFormats/Portable/interface/PortableDeviceCollection.h"

namespace lst {
  using LSTPixelHitsHostCollection = PortableHostCollection<LSTPixelHitsSoA>;
}  // namespace lst

#endif
