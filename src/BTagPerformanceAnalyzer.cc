// -*- C++ -*-
//
// Package:    BTagPerformanceAnalyzer
// Class:      BTagPerformanceAnalyzer
// 
/**\class BTagPerformanceAnalyzer BTagPerformanceAnalyzer.cc MyAnalysis/BTagPerformanceAnalyzer/src/BTagPerformanceAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Dinko Ferencek
//         Created:  Tue Oct 18 13:53:35 CDT 2011
// $Id: BTagPerformanceAnalyzer.cc,v 1.1 2011/11/10 01:00:14 ferencek Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

// ROOT
#include <TH1D.h>
#include <TH2D.h>

// TFileService
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// b-tagging stuff
#include "RecoBTag/Records/interface/BTagPerformanceRecord.h"
#include "CondFormats/PhysicsToolsObjects/interface/BinningPointByMap.h"
#include "RecoBTag/PerformanceDB/interface/BtagPerformance.h"

//
// class declaration
//

class BTagPerformanceAnalyzer : public edm::EDAnalyzer {
   public:
      explicit BTagPerformanceAnalyzer(const edm::ParameterSet&);
      ~BTagPerformanceAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      // ----------member data ---------------------------
      std::vector<std::string> bTagAlgoWP;
      edm::Service<TFileService> fs;
      int PtNBins, etaNBins;
      double PtMin, PtMax, etaMin, etaMax, testPt, testEta;
      std::map<std::string, TH2D*> h2_BTagScaleFactorMap;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
BTagPerformanceAnalyzer::BTagPerformanceAnalyzer(const edm::ParameterSet& iConfig)

{
   //now do what ever initialization is needed
   bTagAlgoWP = iConfig.getParameter< std::vector< std::string > >("BTagAlgoWP");
   PtNBins    = iConfig.getParameter<int>("PtNBins");
   PtMin      = iConfig.getParameter<double>("PtMin");
   PtMax      = iConfig.getParameter<double>("PtMax");
   etaNBins   = iConfig.getParameter<int>("etaNBins");
   etaMin     = iConfig.getParameter<double>("etaMin");
   etaMax     = iConfig.getParameter<double>("etaMax");
   testPt     = iConfig.getParameter<double>("TestPt");
   testEta    = iConfig.getParameter<double>("TestEta");

   for( std::vector<std::string>::const_iterator it = bTagAlgoWP.begin(); it != bTagAlgoWP.end(); ++it )
   {
     h2_BTagScaleFactorMap[*it + "_BTagScaleFactors"]   = fs->make<TH2D>(("h2_" + *it + "_BTagScaleFactors").c_str(),(*it + "_BTagScaleFactors;p_{T} [GeV];#eta").c_str(),PtNBins,PtMin,PtMax,etaNBins,etaMin,etaMax);
     h2_BTagScaleFactorMap[*it + "_MistagScaleFactors"] = fs->make<TH2D>(("h2_" + *it + "_MistagScaleFactors").c_str(),(*it + "_MistagScaleFactors;p_{T} [GeV];#eta").c_str(),PtNBins,PtMin,PtMax,etaNBins,etaMin,etaMax);
   }

}


BTagPerformanceAnalyzer::~BTagPerformanceAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
BTagPerformanceAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   for( std::vector<std::string>::const_iterator it = bTagAlgoWP.begin(); it != bTagAlgoWP.end(); ++it )
   {     
     edm::ESHandle<BtagPerformance> perfBTag;
     iSetup.get<BTagPerformanceRecord>().get("MUJETSWPBTAG" + *it,perfBTag);

     edm::ESHandle<BtagPerformance> perfMistag;
     iSetup.get<BTagPerformanceRecord>().get("MISTAG" + *it,perfMistag);

//      std::cout << "b-tag algo: " << *it << " Working point: " << perfBTag->workingPoint().cut() << std::endl;

     double jetPt, jetAbsEta, btagSF, btagSF_error, mistagSF, mistagSF_error;
     BinningPointByMap p;

     for(int i=0; i<PtNBins; ++i )
     {
       double PtBinWidth = (PtMax - PtMin)/PtNBins;
       jetPt = PtMin + PtBinWidth/2 + PtBinWidth*i;

       for( int j=0; j<etaNBins; ++j )
       {
         double etaBinWidth = (etaMax - etaMin)/etaNBins;
         jetAbsEta = etaMin + etaBinWidth/2 + etaBinWidth*j;

         p.reset();
         p.insert(BinningVariables::JetEta,jetAbsEta);
         p.insert(BinningVariables::JetEt,jetPt);

         btagSF = perfBTag->getResult( PerformanceResult::BTAGBEFFCORR, p );
         btagSF_error = perfBTag->getResult( PerformanceResult::BTAGBERRCORR, p );
         mistagSF = perfMistag->getResult( PerformanceResult::BTAGLEFFCORR, p );
         mistagSF_error = perfMistag->getResult( PerformanceResult::BTAGLERRCORR, p );

         h2_BTagScaleFactorMap[*it + "_BTagScaleFactors"]->SetBinContent(i+1, j+1, (btagSF<0 ? 0 : btagSF));
         h2_BTagScaleFactorMap[*it + "_BTagScaleFactors"]->SetBinError(i+1, j+1, (btagSF_error<0 ? 0 : btagSF_error));
         h2_BTagScaleFactorMap[*it + "_MistagScaleFactors"]->SetBinContent(i+1, j+1, (mistagSF<0 ? 0 : mistagSF));
         h2_BTagScaleFactorMap[*it + "_MistagScaleFactors"]->SetBinError(i+1, j+1, (mistagSF_error<0 ? 0 : mistagSF_error));
       }
     }

     p.reset();
     p.insert(BinningVariables::JetEta,testEta);
     p.insert(BinningVariables::JetEt,testPt);

     std::cout << "Jet Pt = " << testPt <<" GeV, |eta| = " << testEta << " --> Is it OK? " << perfBTag->isResultOk( PerformanceResult::BTAGBEFFCORR, p ) << std::endl
               << "b-tag data/MC scale factor: " << perfBTag->getResult( PerformanceResult::BTAGBEFFCORR, p ) << std::endl
               << "b-tag data/MC scale factor error: " << perfBTag->getResult( PerformanceResult::BTAGBERRCORR, p ) << std::endl;

     std::cout << "Jet Pt = " << testPt <<" GeV, |eta| = " << testEta << " --> Is it OK? " << perfMistag->isResultOk( PerformanceResult::BTAGLEFFCORR, p ) << std::endl
               << "mistag data/MC scale factor: " << perfMistag->getResult( PerformanceResult::BTAGLEFFCORR, p ) << std::endl
               << "mistag data/MC scale factor error: " << perfMistag->getResult( PerformanceResult::BTAGLERRCORR, p ) << std::endl;
   }
   
}


// ------------ method called once each job just before starting event loop  ------------
void 
BTagPerformanceAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
BTagPerformanceAnalyzer::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
void 
BTagPerformanceAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
BTagPerformanceAnalyzer::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
BTagPerformanceAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
BTagPerformanceAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
BTagPerformanceAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BTagPerformanceAnalyzer);
