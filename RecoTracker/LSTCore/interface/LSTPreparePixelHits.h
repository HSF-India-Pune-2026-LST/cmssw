#ifndef RecoTracker_LSTCore_interface_LSTPreparePixelHits_h
#define RecoTracker_LSTCore_interface_LSTPreparePixelHits_h

#include <algorithm>
#include <memory>
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

#include "RecoTracker/LSTCore/interface/Common.h"
#include "RecoTracker/LSTCore/interface/LSTPixelHitsHostCollection.h"

namespace lst {

  template <typename TQueue>
  inline LSTPixelHitsHostCollection preparePixelHits(std::vector<unsigned int> const& pix_detId,
                                                     std::vector<uint16_t> const& pix_clustSizeCol,
                                                     std::vector<uint16_t> const& pix_clustSizeRow,
                                                     std::vector<float> const& pix_x,
                                                     std::vector<float> const& pix_y,
                                                     std::vector<float> const& pix_z,
#ifndef LST_STANDALONE
                                                     std::vector<TrackingRecHit const*> const& pix_hits,
#endif
                                                     TQueue const& queue) {

    int nHits = pix_x.size();

    std::vector<unsigned int> hitIdxs(nHits);
    std::iota(hitIdxs.begin(), hitIdxs.end(), 0);  // probably not needed

    // Build the SoAs

    LSTPixelHitsHostCollection lstPixelHitsHC(queue, nHits);

    auto hits = lstPixelHitsHC.view();
    std::copy_n(pix_x.data(), nHits, hits.xs().data());
    std::copy_n(pix_y.data(), nHits, hits.ys().data());
    std::copy_n(pix_z.data(), nHits, hits.zs().data());
    std::copy_n(pix_detId.data(), nHits, hits.detid().data());
    std::copy_n(pix_clustSizeCol.data(), nHits, hits.clustsizeX().data());
    std::copy_n(pix_clustSizeRow.data(), nHits, hits.clustsizeY().data());
#ifndef LST_STANDALONE
    std::copy_n(pix_hits.data(), nHits, hits.hits().data());
#endif

    std::copy_n(hitIdxs.data(), nHits, hits.idxs().data());

    return lstPixelHitsHC;
  }

}  // namespace lst

#endif
