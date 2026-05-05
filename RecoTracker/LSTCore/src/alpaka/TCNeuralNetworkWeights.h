#ifndef RecoTracker_LSTCore_src_alpaka_TCNeuralNetworkWeights_h
#define RecoTracker_LSTCore_src_alpaka_TCNeuralNetworkWeights_h

#include <alpaka/alpaka.hpp>

#include "FWCore/Utilities/interface/HostDeviceConstant.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE::lst::dnn::tcdnn {

  static constexpr int INPUT = 2;
  static constexpr int HIDDEN = 8;
  static constexpr int OUTPUT = 1;

  HOST_DEVICE_CONSTANT float w1[INPUT][HIDDEN] = {{0.42f, -0.25f, 0.36f, -0.48f, 0.51f, -0.17f, 0.08f, 0.27f},
                                                  {-0.18f, 0.31f, 0.12f, 0.22f, -0.29f, -0.09f, 0.41f, 0.16f}};

  HOST_DEVICE_CONSTANT float b1[HIDDEN] = {0.10f, -0.05f, 0.18f, -0.12f, 0.03f, 0.00f, -0.04f, 0.09f};

  HOST_DEVICE_CONSTANT float w2[HIDDEN][OUTPUT] = {
      {0.33f}, {-0.44f}, {0.21f}, {0.14f}, {0.49f}, {-0.23f}, {0.11f}, {0.19f}};

  HOST_DEVICE_CONSTANT float b2[OUTPUT] = {-0.35f};

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE::lst::dnn::tcdnn
#endif
