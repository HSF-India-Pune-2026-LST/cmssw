#ifndef RecoTracker_LSTCore_interface_alpaka_LSTPixelHitsDeviceCollection_h
#define RecoTracker_LSTCore_interface_alpaka_LSTPixelHitsDeviceCollection_h

#include "DataFormats/Portable/interface/alpaka/PortableCollection.h"

#include "RecoTracker/LSTCore/interface/alpaka/Common.h"
#include "RecoTracker/LSTCore/interface/LSTPixelHitsSoA.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE::lst {
  using LSTPixelHitsDeviceCollection = PortableCollection<LSTPixelHitsSoA>;
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE::lst

#endif
