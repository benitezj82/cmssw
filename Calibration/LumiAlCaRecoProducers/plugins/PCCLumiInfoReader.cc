/**_________________________________________________________________
author: Jose Benitez, Chris Palmer
________________________________________________________________**/
#include <memory>
#include <string>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <iostream> 
#include <map>
#include <utility>
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/Luminosity/interface/LumiCorrections.h"
#include "CondFormats/DataRecord/interface/LumiCorrectionsRcd.h"
#include "CondFormats/Serialization/interface/Serializable.h"
#include "DataFormats/Luminosity/interface/PixelClusterCounts.h"
#include "DataFormats/Luminosity/interface/LumiInfo.h"
#include "DataFormats/Luminosity/interface/LumiConstants.h"
#include "DataFormats/Provenance/interface/LuminosityBlockRange.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/FileBlock.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/IOVSyncValue.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Run.h"

class PCCLumiInfoReader : public edm::one::EDProducer<edm::EndRunProducer,edm::one::WatchRuns,edm::EndLuminosityBlockProducer,edm::one::WatchLuminosityBlocks> {
    public:
        explicit PCCLumiInfoReader(const edm::ParameterSet&);
        ~PCCLumiInfoReader();

    private:
        virtual void beginRun(edm::Run const& runSeg, const edm::EventSetup& iSetup) override final;
        virtual void beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup) override final;
        virtual void endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup) override final;
        virtual void endRun(edm::Run const& runSeg, const edm::EventSetup& iSetup) override final;
        virtual void endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) override final;
        virtual void endRunProduce(edm::Run& runSeg, const edm::EventSetup& iSetup) override final;
        virtual void endJob()  override final;
        virtual void produce                  (edm::Event& iEvent, const edm::EventSetup& iSetup) override final;

  edm::EDGetTokenT<LumiInfo>  lumiInfoToken;
  std::string   pccSrc_;//input file EDproducer module label
  std::string   prodInst_;//input file product instance
  
  unsigned int countLumi_;//lb's
  unsigned int run_;//keep track of the run number


  std::ofstream csvfile;  

};

//--------------------------------------------------------------------------------------------------
PCCLumiInfoReader::PCCLumiInfoReader(const edm::ParameterSet& iConfig)
{

  pccSrc_ = iConfig.getParameter<std::string>("inLumiObLabel"); 
  prodInst_ = iConfig.getParameter<std::string>("ProdInst");
  
  // approxLumiBlockSize_=iConfig.getParameter<edm::ParameterSet>("PCCLumiInfoReaderParameters").getParameter<int>("approxLumiBlockSize");
  // type2_a_ = iConfig.getParameter<edm::ParameterSet>("PCCLumiInfoReaderParameters").getParameter<double>("type2_a");
  
  run_=0;
  countLumi_=0;
  
  edm::InputTag inputPCCTag_(pccSrc_, prodInst_);
  lumiInfoToken=consumes<LumiInfo, edm::InLumi>(inputPCCTag_);

}

//--------------------------------------------------------------------------------------------------
PCCLumiInfoReader::~PCCLumiInfoReader(){
}


//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){}


//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::beginRun(edm::Run const& runSeg, const edm::EventSetup& iSetup){
  run_=runSeg.run();
}
//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup){
  countLumi_++;
  
  edm::Handle<LumiInfo> PCCHandle; 
  lumiSeg.getByToken(lumiInfoToken,PCCHandle);
  const LumiInfo& inLumiOb = *(PCCHandle.product()); 
  
  const std::vector<float> lumiByBX= inLumiOb.getInstLumiAllBX();

  csvfile.open("PCCLumiInfoReader.csv", std::ios_base::app);
  csvfile<<std::to_string(lumiSeg.run())<<",";
  csvfile<<std::to_string(lumiSeg.luminosityBlock())<<",";
  
  //std::cout<<countLumi_<<","<<run_<<","<<lumiSeg.luminosityBlock()<<","<<inLumiOb.getTotalInstLumi()<<","<<lumiByBX.size()<<std::endl;
  
  csvfile<<std::to_string(inLumiOb.getTotalInstLumi());
  for(unsigned int i=0;i<lumiByBX.size();i++)
    csvfile<<","<<std::to_string(lumiByBX[i]);
  
  csvfile<<std::endl;
  
  csvfile.close();

}


//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup){
  

}

//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::endRun(edm::Run const& runSeg, const edm::EventSetup& iSetup){
}

//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup){
}

//--------------------------------------------------------------------------------------------------
void PCCLumiInfoReader::endRunProduce(edm::Run& runSeg, const edm::EventSetup& iSetup){
 
}

void PCCLumiInfoReader::endJob(){
}

DEFINE_FWK_MODULE(PCCLumiInfoReader);
