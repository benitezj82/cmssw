#include <cuda_runtime.h>

#include "CUDADataFormats/BeamSpot/interface/BeamSpotCUDA.h"
#include "CUDADataFormats/SiPixelCluster/interface/SiPixelClustersCUDA.h"
#include "CUDADataFormats/SiPixelDigi/interface/SiPixelDigisCUDA.h"
#include "CUDADataFormats/TrackingRecHit/interface/TrackingRecHit2DHeterogeneous.h"
#include "CUDADataFormats/Common/interface/HostProduct.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonTopologies/interface/SimplePixelTopology.h"
#include "RecoLocalTracker/Records/interface/TkPixelCPERecord.h"
#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPEBase.h"
#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPEFast.h"

#include "gpuPixelRecHits.h"

class SiPixelRecHitSoAFromLegacy : public edm::global::EDProducer<> {
public:
  explicit SiPixelRecHitSoAFromLegacy(const edm::ParameterSet& iConfig);
  ~SiPixelRecHitSoAFromLegacy() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  using HitModuleStart = std::array<uint32_t, gpuClustering::maxNumModules + 1>;
  using HMSstorage = HostProduct<uint32_t[]>;

private:
  void produce(edm::StreamID streamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;

  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomToken_;
  const edm::ESGetToken<PixelClusterParameterEstimator, TkPixelCPERecord> cpeToken_;
  const edm::EDGetTokenT<reco::BeamSpot> bsGetToken_;
  const edm::EDGetTokenT<SiPixelClusterCollectionNew> clusterToken_;  // Legacy Clusters
  const edm::EDPutTokenT<TrackingRecHit2DCPU> tokenHit_;
  const edm::EDPutTokenT<HMSstorage> tokenModuleStart_;
  const bool convert2Legacy_;
  const bool isPhase2_;
};

SiPixelRecHitSoAFromLegacy::SiPixelRecHitSoAFromLegacy(const edm::ParameterSet& iConfig)
    : geomToken_(esConsumes()),
      cpeToken_(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("CPE")))),
      bsGetToken_{consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpot"))},
      clusterToken_{consumes<SiPixelClusterCollectionNew>(iConfig.getParameter<edm::InputTag>("src"))},
      tokenHit_{produces<TrackingRecHit2DCPU>()},
      tokenModuleStart_{produces<HMSstorage>()},
      convert2Legacy_(iConfig.getParameter<bool>("convertToLegacy")),
      isPhase2_(iConfig.getParameter<bool>("isPhase2")) {
  if (convert2Legacy_)
    produces<SiPixelRecHitCollectionNew>();
}

void SiPixelRecHitSoAFromLegacy::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("beamSpot", edm::InputTag("offlineBeamSpot"));
  desc.add<edm::InputTag>("src", edm::InputTag("siPixelClustersPreSplitting"));
  desc.add<std::string>("CPE", "PixelCPEFast");
  desc.add<bool>("convertToLegacy", false);
  desc.add<bool>("isPhase2", false);
  descriptions.addWithDefaultLabel(desc);
}

void SiPixelRecHitSoAFromLegacy::produce(edm::StreamID streamID, edm::Event& iEvent, const edm::EventSetup& es) const {
  const TrackerGeometry* geom_ = &es.getData(geomToken_);
  PixelCPEFast const* fcpe = dynamic_cast<const PixelCPEFast*>(&es.getData(cpeToken_));
  if (not fcpe) {
    throw cms::Exception("Configuration") << "SiPixelRecHitSoAFromLegacy can only use a CPE of type PixelCPEFast";
  }
  auto const& cpeView = fcpe->getCPUProduct();

  const reco::BeamSpot& bs = iEvent.get(bsGetToken_);

  BeamSpotPOD bsHost;
  bsHost.x = bs.x0();
  bsHost.y = bs.y0();
  bsHost.z = bs.z0();

  edm::Handle<SiPixelClusterCollectionNew> hclusters;
  iEvent.getByToken(clusterToken_, hclusters);
  auto const& input = *hclusters;

  const int nMaxModules = isPhase2_ ? phase2PixelTopology::numberOfModules : phase1PixelTopology::numberOfModules;
  const int startBPIX2 = isPhase2_ ? phase2PixelTopology::layerStart[1] : phase1PixelTopology::layerStart[1];

  assert(nMaxModules < gpuClustering::maxNumModules);
  assert(startBPIX2 < nMaxModules);

  // allocate a buffer for the indices of the clusters
  auto hmsp = std::make_unique<uint32_t[]>(nMaxModules + 1);
  // hitsModuleStart is a non-owning pointer to the buffer
  auto hitsModuleStart = hmsp.get();
  // wrap the buffer in a HostProduct
  auto hms = std::make_unique<HMSstorage>(std::move(hmsp));
  // move the HostProduct to the Event, without reallocating the buffer or affecting hitsModuleStart
  iEvent.put(tokenModuleStart_, std::move(hms));

  // legacy output
  auto legacyOutput = std::make_unique<SiPixelRecHitCollectionNew>();

  // storage
  std::vector<uint16_t> xx;
  std::vector<uint16_t> yy;
  std::vector<uint16_t> adc;
  std::vector<uint16_t> moduleInd;
  std::vector<int32_t> clus;

  std::vector<edm::Ref<edmNew::DetSetVector<SiPixelCluster>, SiPixelCluster>> clusterRef;

  constexpr uint32_t maxHitsInModule = gpuClustering::maxHitsInModule();

  HitModuleStart moduleStart_;  // index of the first pixel of each module
  HitModuleStart clusInModule_;
  memset(&clusInModule_, 0, sizeof(HitModuleStart));  // needed??
  memset(&moduleStart_, 0, sizeof(HitModuleStart));
  assert(gpuClustering::maxNumModules + 1 == clusInModule_.size());
  assert(0 == clusInModule_[gpuClustering::maxNumModules]);
  uint32_t moduleId_;
  moduleStart_[1] = 0;  // we run sequentially....

  SiPixelClustersCUDA::SiPixelClustersCUDASOAView clusterView{
      moduleStart_.data(), clusInModule_.data(), &moduleId_, hitsModuleStart};

  // fill cluster arrays
  int numberOfClusters = 0;
  for (auto const& dsv : input) {
    unsigned int detid = dsv.detId();
    DetId detIdObject(detid);
    const GeomDetUnit* genericDet = geom_->idToDetUnit(detIdObject);
    auto gind = genericDet->index();
    assert(gind < nMaxModules);
    auto const nclus = dsv.size();
    clusInModule_[gind] = nclus;
    numberOfClusters += nclus;
  }
  hitsModuleStart[0] = 0;

  for (int i = 1, n = nMaxModules + 1; i < n; ++i)
    hitsModuleStart[i] = hitsModuleStart[i - 1] + clusInModule_[i - 1];

  assert(numberOfClusters == int(hitsModuleStart[nMaxModules]));

  // output SoA
  // element 96 is the start of BPIX2 (i.e. the number of clusters in BPIX1)

  auto output = std::make_unique<TrackingRecHit2DCPU>(
      numberOfClusters, isPhase2_, hitsModuleStart[startBPIX2], &cpeView, hitsModuleStart, nullptr);
  assert(output->nMaxModules() == uint32_t(nMaxModules));

  if (0 == numberOfClusters) {
    iEvent.put(std::move(output));
    if (convert2Legacy_)
      iEvent.put(std::move(legacyOutput));
    return;
  }

  if (convert2Legacy_)
    legacyOutput->reserve(nMaxModules, numberOfClusters);

  int numberOfDetUnits = 0;
  int numberOfHits = 0;
  for (auto const& dsv : input) {
    numberOfDetUnits++;
    unsigned int detid = dsv.detId();
    DetId detIdObject(detid);
    const GeomDetUnit* genericDet = geom_->idToDetUnit(detIdObject);
    auto const gind = genericDet->index();
    assert(gind < nMaxModules);
    const PixelGeomDetUnit* pixDet = dynamic_cast<const PixelGeomDetUnit*>(genericDet);
    assert(pixDet);
    auto const nclus = dsv.size();
    assert(clusInModule_[gind] == nclus);
    if (0 == nclus)
      continue;  // is this really possible?

    auto const fc = hitsModuleStart[gind];
    auto const lc = hitsModuleStart[gind + 1];
    assert(lc > fc);
    LogDebug("SiPixelRecHitSoAFromLegacy") << "in det " << gind << ": conv " << nclus << " hits from " << dsv.size()
                                           << " legacy clusters" << ' ' << fc << ',' << lc;
    assert((lc - fc) == nclus);
    if (nclus > maxHitsInModule)
      printf(
          "WARNING: too many clusters %d in Module %d. Only first %d Hits converted\n", nclus, gind, maxHitsInModule);

    // fill digis
    xx.clear();
    yy.clear();
    adc.clear();
    moduleInd.clear();
    clus.clear();
    clusterRef.clear();
    moduleId_ = gind;
    uint32_t ic = 0;
    uint32_t ndigi = 0;
    for (auto const& clust : dsv) {
      assert(clust.size() > 0);
      for (int i = 0, nd = clust.size(); i < nd; ++i) {
        auto px = clust.pixel(i);
        xx.push_back(px.x);
        yy.push_back(px.y);
        adc.push_back(px.adc);
        moduleInd.push_back(gind);
        clus.push_back(ic);
        ++ndigi;
      }
      assert(clust.originalId() == ic);  // make sure hits and clus are in sync
      if (convert2Legacy_)
        clusterRef.emplace_back(edmNew::makeRefTo(hclusters, &clust));
      ic++;
    }
    assert(nclus == ic);
    assert(clus.size() == ndigi);
    numberOfHits += nclus;
    // filled creates view
    SiPixelDigisCUDASOAView digiView;
    digiView.xx_ = xx.data();
    digiView.yy_ = yy.data();
    digiView.adc_ = adc.data();
    digiView.moduleInd_ = moduleInd.data();
    digiView.clus_ = clus.data();
    digiView.pdigi_ = nullptr;
    digiView.rawIdArr_ = nullptr;
    assert(digiView.adc(0) != 0);
    // we run on blockId.x==0
    gpuPixelRecHits::getHits(&cpeView, &bsHost, digiView, ndigi, &clusterView, output->view());
    for (auto h = fc; h < lc; ++h)
      if (h - fc < maxHitsInModule)
        assert(gind == output->view()->detectorIndex(h));
      else
        assert(gpuClustering::invalidModuleId == output->view()->detectorIndex(h));
    if (convert2Legacy_) {
      SiPixelRecHitCollectionNew::FastFiller recHitsOnDetUnit(*legacyOutput, detid);
      for (auto h = fc; h < lc; ++h) {
        auto ih = h - fc;

        if (ih >= maxHitsInModule)
          break;
        assert(ih < clusterRef.size());
        LocalPoint lp(output->view()->xLocal(h), output->view()->yLocal(h));
        LocalError le(output->view()->xerrLocal(h), 0, output->view()->yerrLocal(h));
        SiPixelRecHitQuality::QualWordType rqw = 0;
        SiPixelRecHit hit(lp, le, rqw, *genericDet, clusterRef[ih]);
        recHitsOnDetUnit.push_back(hit);
      }
    }
  }

  assert(numberOfHits == numberOfClusters);

  // fill data structure to support CA
  const auto nLayers = isPhase2_ ? phase2PixelTopology::numberOfLayers : phase1PixelTopology::numberOfLayers;
  for (auto i = 0U; i < nLayers + 1; ++i) {
    output->hitsLayerStart()[i] = hitsModuleStart[cpeView.layerGeometry().layerStart[i]];
    LogDebug("SiPixelRecHitSoAFromLegacy")
        << "Layer n." << i << " - starting at module: " << cpeView.layerGeometry().layerStart[i]
        << " - starts ad cluster: " << output->hitsLayerStart()[i] << "\n";
  }

  cms::cuda::fillManyFromVector(output->phiBinner(),
                                nLayers,
                                output->iphi(),
                                output->hitsLayerStart(),
                                numberOfHits,
                                256,
                                output->phiBinnerStorage());

  LogDebug("SiPixelRecHitSoAFromLegacy") << "created HitSoa for " << numberOfClusters << " clusters in "
                                         << numberOfDetUnits << " Dets";
  iEvent.put(std::move(output));
  if (convert2Legacy_)
    iEvent.put(std::move(legacyOutput));
}

DEFINE_FWK_MODULE(SiPixelRecHitSoAFromLegacy);
