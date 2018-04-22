// -*- C++ -*-
//
// Original Author:  Jose Feliciano Benitez
//         Created:  Mon, 16 Apr 2018 02:53:46 GMT
//

// system include files
#include <memory>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Luminosity/interface/LumiInfo.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

//
// class declaration
//
class PCCLumiAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchLuminosityBlocks>  {

public:
  explicit PCCLumiAnalyzer(const edm::ParameterSet&);
  ~PCCLumiAnalyzer();
  
private:
  
  virtual void beginJob() override; 
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override {};

  virtual void analyze(const edm::Event&, const edm::EventSetup&) override {};//this module does not use event-by-event info 

  edm::EDGetTokenT<LumiInfo>  lumiInfoToken;
  std::string   pccSrc_;//input file EDproducer module label
  std::string   prodInst_;//input file product instance

  unsigned int countLumi_;//lb's
  unsigned int run_;//keep track of the run number

  std::string   csvfilename_;//
  std::ofstream csvfile;
};


//
// constructors and destructor
//
PCCLumiAnalyzer::PCCLumiAnalyzer(const edm::ParameterSet& iConfig)
  :
  countLumi_(0), 
  run_(0),
  csvfilename_("PCCLumiByBX.csv")
{
  //
  //read the config
  //
  pccSrc_ = iConfig.getParameter<std::string>("inLumiObLabel");
  prodInst_ = iConfig.getParameter<std::string>("ProdInst");

  //
  //create LumiInfo token
  //
  edm::InputTag inputPCCTag_(pccSrc_, prodInst_);
  lumiInfoToken=consumes<LumiInfo, edm::InLumi>(inputPCCTag_);

}


PCCLumiAnalyzer::~PCCLumiAnalyzer(){}


//
// member functions
//
void 
PCCLumiAnalyzer::beginJob()
{
  ////
  ///check if its possible to create the output file
  ////
  system((std::string("rm -f ")+csvfilename_).c_str());
  csvfile.open(csvfilename_, std::ios_base::out);
  if (!csvfile.is_open())
    //std::cout << "Error opening file"<<std::endl;
    throw cms::Exception("PCCLumiAnalyzer:: ") <<  " unable to create the csv file.";
  csvfile.close();
  

}

//--------------------------------------------------------------------------------------------------
void 
PCCLumiAnalyzer::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup)
{
  countLumi_++;

  //
  //get the lumi info 
  //
  edm::Handle<LumiInfo> PCCHandle;
  lumiSeg.getByToken(lumiInfoToken,PCCHandle);
  const LumiInfo& inLumiOb = *(PCCHandle.product());
  const std::vector<float> lumiByBX= inLumiOb.getInstLumiAllBX();

  //
  //Open the output file
  //
  csvfile.open(csvfilename_, std::ios_base::app);
  csvfile<<std::to_string(lumiSeg.run())<<",";
  csvfile<<std::to_string(lumiSeg.luminosityBlock())<<",";

  //
  //fill the output file
  //
  //std::cout<<countLumi_<<","<<run_<<","<<lumiSeg.luminosityBlock()<<","<<inLumiOb.getTotalInstLumi()<<","<<lumiByBX.size()<<std::endl;
  csvfile<<std::to_string(inLumiOb.getTotalInstLumi());
  //for(unsigned int i=0;i<lumiByBX.size();i++)
  for(unsigned int i=0;i<10;i++)
    csvfile<<","<<std::to_string(lumiByBX[i]);
  csvfile<<std::endl;


  csvfile.close();
}

//define this as a plug-in
DEFINE_FWK_MODULE(PCCLumiAnalyzer);
