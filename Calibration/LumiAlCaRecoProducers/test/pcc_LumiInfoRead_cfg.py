#########################
#Test LumiInfo reader
#########################
import FWCore.ParameterSet.Config as cms

process = cms.Process("PCCLumiAnalyzer")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100000) )

process.source = cms.Source(
    "PoolSource",fileNames = cms.untracked.vstring(
        #'file:/eos/cms/store/group/comm_luminosity/PCC/ForLumiSystematics/2017/5Feb2018/AlCaLumiPixels/PCC_AlCaLumiPixels_Run2017B_5kLS/180205_190216/0000/raw_corr_PCC_RD_1.root',
        #'file:/eos/cms/store/group/comm_luminosity/PCC/ForLumiComputations/2017/5Feb2018/AlCaLumiPixels/PCC_AlCaLumiPixels_Run2017C_1kLS_NoZeroes/180207_184738/0000/rawPCC_297411_ZB_112.root',
        'file:./rawPCC_297411_ZB_112.root',
        )                 
    )


#############################    init the reader   #######################################
process.PCCLumiAnalyzer = cms.EDAnalyzer("PCCLumiAnalyzer", 
        cms.PSet(
        # inLumiObLabel = cms.string("rawPCCProd"),ProdInst = cms.string("rawPCCRandom"),
        # LuminosityBlocks->Print()    ...   LumiInfo_rawPCCProd_rawPCZeroBias_rawRECO.obj.errLumiByBX_
        inLumiObLabel = cms.string("rawPCCProd"), ProdInst = cms.string("rawPCZeroBias"),
        )
    )

process.p = cms.Path(process.PCCLumiAnalyzer)


############################# MessageLogger #######################################
process.MessageLogger = cms.Service("MessageLogger",
    FrameworkJobReport = cms.untracked.PSet(
        FwkJob = cms.untracked.PSet(
            limit = cms.untracked.int32(10000000),
            optionalPSet = cms.untracked.bool(True)
        ),
        default = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        optionalPSet = cms.untracked.bool(True)
    ),
    categories = cms.untracked.vstring('FwkJob', 
        'FwkReport', 
        'FwkSummary', 
        'Root_NoDictionary'),
    cerr = cms.untracked.PSet(
        FwkJob = cms.untracked.PSet(
            limit = cms.untracked.int32(0),
            optionalPSet = cms.untracked.bool(True)
        ),
        FwkReport = cms.untracked.PSet(
            limit = cms.untracked.int32(10000000),
            optionalPSet = cms.untracked.bool(True),
            reportEvery = cms.untracked.int32(100000)
        ),
        FwkSummary = cms.untracked.PSet(
            limit = cms.untracked.int32(10000000),
            optionalPSet = cms.untracked.bool(True),
            reportEvery = cms.untracked.int32(10000)
        ),
        INFO = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        Root_NoDictionary = cms.untracked.PSet(
            limit = cms.untracked.int32(0),
            optionalPSet = cms.untracked.bool(True)
        ),
        default = cms.untracked.PSet(
            limit = cms.untracked.int32(10000000)
        ),
        noTimeStamps = cms.untracked.bool(False),
        optionalPSet = cms.untracked.bool(True),
        threshold = cms.untracked.string('INFO')
    ),
    cerr_stats = cms.untracked.PSet(
        optionalPSet = cms.untracked.bool(True),
        output = cms.untracked.string('cerr'),
        threshold = cms.untracked.string('WARNING')
    ),
    cout = cms.untracked.PSet(
        placeholder = cms.untracked.bool(True)
    ),
    debugModules = cms.untracked.vstring(),
    debugs = cms.untracked.PSet(
        placeholder = cms.untracked.bool(True)
    ),
    default = cms.untracked.PSet(

    ),
    destinations = cms.untracked.vstring('warnings', 
        'errors', 
        'infos', 
        'debugs', 
        'cout', 
        'cerr'),
    errors = cms.untracked.PSet(
        placeholder = cms.untracked.bool(True)
    ),
    fwkJobReports = cms.untracked.vstring('FrameworkJobReport'),
    infos = cms.untracked.PSet(
        #Root_NoDictionary = cms.untracked.PSet(
        #    limit = cms.untracked.int32(0),
        #    optionalPSet = cms.untracked.bool(True)
        #),
        #optionalPSet = cms.untracked.bool(True),
        #placeholder = cms.untracked.bool(True)
        threshold = cms.untracked.string('INFO')
    ),
    statistics = cms.untracked.vstring('cerr_stats'),
    suppressDebug = cms.untracked.vstring(),
    suppressInfo = cms.untracked.vstring(),
    suppressWarning = cms.untracked.vstring(),
    warnings = cms.untracked.PSet(
        #placeholder = cms.untracked.bool(True)
        threshold = cms.untracked.string('WARNING')
    )
)
#added line for additional output summary `
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True))

#process.outpath = cms.EndPath(process.out)
