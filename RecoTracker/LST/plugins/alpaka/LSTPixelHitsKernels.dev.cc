#include "LSTPixelHitsKernels.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  namespace {
    struct CopyHits {
      ALPAKA_FN_ACC void operator()(Acc1D const& acc,
                                    ::reco::TrackingRecHitConstView inHits,
                                    ::lst::LSTPixelHitsView outHits, int nHits) const {
        for (unsigned int i : cms::alpakatools::uniform_elements(acc, nHits)) {
          printf("CopyHits: %dof %d \n", i, nHits);
          outHits.ys()[i] = inHits.yGlobal()[i];
        }
      }
    };
  }
  
  void copyHits(Queue& queue, ::reco::TrackingRecHitConstView input, ::lst::LSTPixelHitsView output) {
    std::cout << "copyHits kernel: input size=" << input.metadata().size()
              << ", queue=" << &queue << std::endl;

#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED
    #include <cuda_runtime.h>

    // Check if output.ys() is on device
    cudaPointerAttributes attr_ys;
    auto ys_ptr = output.ys().data();
    cudaPointerGetAttributes(&attr_ys, ys_ptr);
    std::cout << "output.ys() pointer: " << ys_ptr
              << ", type: " << (attr_ys.type == cudaMemoryTypeDevice ? "DEVICE" :
                                attr_ys.type == cudaMemoryTypeHost ? "HOST" : "UNKNOWN") << std::endl;

    // Check if input.yGlobal() is on device
    cudaPointerAttributes attr_yg;
    auto yg_ptr = input.yGlobal().data();
    cudaPointerGetAttributes(&attr_yg, yg_ptr);
    std::cout << "input.yGlobal() pointer: " << yg_ptr
              << ", type: " << (attr_yg.type == cudaMemoryTypeDevice ? "DEVICE" :
                                attr_yg.type == cudaMemoryTypeHost ? "HOST" : "UNKNOWN") << std::endl;
    #endif
    
    
    auto const workDiv = cms::alpakatools::make_workdiv<Acc1D>(1, std::min(1024, input.metadata().size()));
    alpaka::exec<Acc1D>(queue, workDiv, CopyHits{}, input, output, input.metadata().size());

    std::cout << "copyHits kernel launched" << std::endl;
    alpaka::wait(queue);  // Wait for kernel to complete before emplace
    std::cout << "After CopyHits - completed successfully" << std::endl;
  }

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE
