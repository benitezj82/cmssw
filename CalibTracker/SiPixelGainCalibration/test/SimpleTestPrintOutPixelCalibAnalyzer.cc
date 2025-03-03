// -*- C++ -*-
//
// Package:    SimpleTestPrintOutPixelCalibAnalyzer
// Class:      SimpleTestPrintOutPixelCalibAnalyzer
//
/**\class SimpleTestPrintOutPixelCalibAnalyzer CalibTracker/SiPixelGainCalibration/test/SimpleTestPrintOutPixelCalibAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Freya Blekman
//         Created:  Mon Nov  5 16:56:35 CET 2007
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiPixelDigi/interface/SiPixelCalibDigi.h"
#include "CalibTracker/SiPixelGainCalibration/test/SimpleTestPrintOutPixelCalibAnalyzer.h"

//
// constructors and destructor
//
SimpleTestPrintOutPixelCalibAnalyzer::SimpleTestPrintOutPixelCalibAnalyzer(const edm::ParameterSet& iConfig) {
  tPixelCalibDigi = consumes<edm::DetSetVector<SiPixelCalibDigi> >(edm::InputTag("siPixelCalibDigis"));
}

//
// member functions
//
void SimpleTestPrintOutPixelCalibAnalyzer::printInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup) const {
  using namespace edm;

  Handle<DetSetVector<SiPixelCalibDigi> > pIn;
  iEvent.getByToken(tPixelCalibDigi, pIn);

  DetSetVector<SiPixelCalibDigi>::const_iterator digiIter;
  for (digiIter = pIn->begin(); digiIter != pIn->end(); ++digiIter) {
    uint32_t detid = digiIter->id;
    DetSet<SiPixelCalibDigi>::const_iterator ipix;
    for (ipix = digiIter->data.begin(); ipix != digiIter->end(); ++ipix) {
      edm::LogPrint("SimpleTestPrintOutPixelCalibAnalyzer") << std::endl;
      for (uint32_t ipoint = 0; ipoint < ipix->getnpoints(); ++ipoint)
        edm::LogPrint("SimpleTestPrintOutPixelCalibAnalyzer")
            << "\t Det ID " << detid << " row:" << ipix->row() << " col:" << ipix->col() << " point " << ipoint
            << " has " << ipix->getnentries(ipoint) << " entries, adc: " << ipix->getsum(ipoint)
            << ", adcsq: " << ipix->getsumsquares(ipoint) << std::endl;
    }
  }
}
// ------------ method called to for each event  ------------
void SimpleTestPrintOutPixelCalibAnalyzer::analyze(edm::StreamID id,
                                                   edm::Event const& iEvent,
                                                   edm::EventSetup const& iSetup) const {
  using namespace edm;
  printInfo(iEvent, iSetup);
}
