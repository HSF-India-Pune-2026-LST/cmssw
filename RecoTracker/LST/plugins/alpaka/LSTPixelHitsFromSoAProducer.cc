#include "DataFormats/TrackingRecHitSoA/interface/TrackingRecHitsDevice.h"
#include "DataFormats/TrackingRecHitSoA/interface/alpaka/TrackingRecHitsSoACollection.h"
#include "RecoTracker/LSTCore/interface/alpaka/LSTPixelHitsDeviceCollection.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/Event.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EventSetup.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/global/EDProducer.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"

#include "LSTPixelHitsKernels.h"

//#define GPU_DEBUG

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  class LSTPixelHitsFromSoAProducer : public global::EDProducer<> {
  public:
    explicit LSTPixelHitsFromSoAProducer(const edm::ParameterSet& iConfig);
    ~LSTPixelHitsFromSoAProducer() override = default;

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    void produce(edm::StreamID streamID, device::Event& iEvent, const device::EventSetup& iSetup) const override;

    const device::EDGetToken<reco::TrackingRecHitsSoACollection> pixelInputToken_;

    const device::EDPutToken<lst::LSTPixelHitsDeviceCollection> pixelOutput_;
  };

  LSTPixelHitsFromSoAProducer::LSTPixelHitsFromSoAProducer(const edm::ParameterSet& iConfig)
      : EDProducer(iConfig),
        pixelInputToken_(consumes(iConfig.getParameter<edm::InputTag>("pixelRecHitsSoA"))),
        pixelOutput_(produces()) {}

  void LSTPixelHitsFromSoAProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;

    desc.add<edm::InputTag>("pixelRecHitsSoA", edm::InputTag("siPixelRecHitsPreSplittingAlpaka"));

    descriptions.addWithDefaultLabel(desc);
  }

  void LSTPixelHitsFromSoAProducer::produce(edm::StreamID streamID,
                                            device::Event& iEvent,
                                            const device::EventSetup& es) const {
    // get both Pixel and Tracker SoA collections
    auto& queue = iEvent.queue();
    const auto& pixColl = iEvent.get(pixelInputToken_);
    const int nPixHits = pixColl.nHits();

    // the output is also a SoA with a simpler layout
    auto output = lst::LSTPixelHitsDeviceCollection(queue, nPixHits);

    // start from the hits SoA
    auto outView = output.view();
    auto pixView = pixColl.view().trackingHits();

    // copy xGlobal to xs
    alpaka::memcpy(
        queue,
        cms::alpakatools::make_device_view(queue, outView.xs().data(), nPixHits),
        cms::alpakatools::make_device_view(queue, pixView.xGlobal().data(), nPixHits));

    std::cout << "Before copyHits:" << std::endl;
    std::cout << "  Queue: " << &queue << std::endl;
    std::cout << "  nPixHits: " << nPixHits << std::endl;
    std::cout << "  pixView.metadata().size(): " << pixView.metadata().size() << std::endl;
    std::cout << "  outView.xs().data(): " << outView.xs().data() << std::endl;

    try {
    copyHits(queue, pixView, outView);
    } catch (const std::exception& e) {
      std::cerr << "copyHits exception: " << e.what() << std::endl;
      throw;
    }
    alpaka::wait(queue);  // Wait for kernel to complete before emplace
    std::cout << "After copyHits - completed successfully" << std::endl;
    // emplace the output
    iEvent.emplace(pixelOutput_, std::move(output));
  }
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

#include "HeterogeneousCore/AlpakaCore/interface/alpaka/MakerMacros.h"
DEFINE_FWK_ALPAKA_MODULE(LSTPixelHitsFromSoAProducer);
