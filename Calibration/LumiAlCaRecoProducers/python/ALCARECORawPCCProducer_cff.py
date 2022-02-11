import FWCore.ParameterSet.Config as cms

from Calibration.LumiAlCaRecoProducers.alcaRawPCCProducer_cfi import *
ALCARECORawPCCProd = rawPCCProd.clone()
ALCARECORawPCCProd.RawPCCProducerParameters.inputPccLabel="alcaPCCIntegratorZeroBias"
ALCARECORawPCCProd.RawPCCProducerParameters.ProdInst="alcaPCCZeroBias"
ALCARECORawPCCProd.RawPCCProducerParameters.outputProductName="rawPCCProd"
ALCARECORawPCCProd.RawPCCProducerParameters.ApplyCorrections=True
ALCARECORawPCCProd.RawPCCProducerParameters.saveCSVFile=False  # .csv file may be retrived from PromtReco Crab jobs (saved into dataset)

seqALCARECORawPCCProducer = cms.Sequence(ALCARECORawPCCProd)
