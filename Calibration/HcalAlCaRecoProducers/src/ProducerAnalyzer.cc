// system include files

// user include files

#include "Calibration/HcalAlCaRecoProducers/src/ProducerAnalyzer.h"
#include "DataFormats/Provenance/interface/StableProvenance.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h" 
#include "Geometry/CaloGeometry/interface/CaloGeometry.h" 
#include "Geometry/HcalTowerAlgo/interface/HcalGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h" 
#include "DataFormats/HcalCalibObjects/interface/HOCalibVariables.h"
#include "DataFormats/TrackReco/interface/TrackExtra.h"
#include "DataFormats/TrackReco/interface/TrackExtraFwd.h"
#include "RecoTracker/TrackProducer/interface/TrackProducerBase.h"

using namespace reco;

namespace cms
{

//
// constructors and destructor
//
ProducerAnalyzer::ProducerAnalyzer(const edm::ParameterSet& iConfig)
{
  // get name of output file with histogramms  
   
   nameProd_ = iConfig.getUntrackedParameter<std::string>("nameProd");
   jetCalo_ = iConfig.getUntrackedParameter<std::string>("jetCalo","GammaJetJetBackToBackCollection");
   gammaClus_ = iConfig.getUntrackedParameter<std::string>("gammaClus","GammaJetGammaBackToBackCollection");
   ecalInput_=iConfig.getUntrackedParameter<std::string>("ecalInput","GammaJetEcalRecHitCollection");
   hbheInput_ = iConfig.getUntrackedParameter<std::string>("hbheInput");
   hoInput_ = iConfig.getUntrackedParameter<std::string>("hoInput");
   hfInput_ = iConfig.getUntrackedParameter<std::string>("hfInput");
   Tracks_ = iConfig.getUntrackedParameter<std::string>("Tracks","GammaJetTracksCollection");    

   tok_hovar_ = consumes<HOCalibVariableCollection>( edm::InputTag(nameProd_,hoInput_) );
   tok_horeco_ = consumes<HORecHitCollection>( edm::InputTag("horeco") );
   tok_ho_ = consumes<HORecHitCollection>( edm::InputTag(hoInput_) );
   tok_hoProd_ = consumes<HORecHitCollection>( edm::InputTag(nameProd_,hoInput_) );

    tok_hf_ = consumes<HFRecHitCollection>( edm::InputTag(hfInput_) );

   tok_jets_ = consumes<reco::CaloJetCollection>( edm::InputTag(nameProd_,jetCalo_) );
   tok_gamma_ = consumes<reco::SuperClusterCollection>( edm::InputTag(nameProd_,gammaClus_) );
   tok_muons_ = consumes<reco::MuonCollection>(edm::InputTag(nameProd_,"SelectedMuons"));
   tok_ecal_ = consumes<EcalRecHitCollection>( edm::InputTag(nameProd_,ecalInput_) );
   tok_tracks_ = consumes<reco::TrackCollection>( edm::InputTag(nameProd_,Tracks_) );

   tok_hbheProd_ = consumes<HBHERecHitCollection>( edm::InputTag(nameProd_,hbheInput_) );
   tok_hbhe_ = consumes<HBHERecHitCollection>( edm::InputTag(hbheInput_) );

}

ProducerAnalyzer::~ProducerAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

void ProducerAnalyzer::beginJob()
{
}

void ProducerAnalyzer::endJob()
{
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
ProducerAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  using namespace edm;

  edm::ESHandle<CaloGeometry> pG;
  iSetup.get<CaloGeometryRecord>().get(pG);
  const CaloGeometry* geo = pG.product();
   

  std::vector<StableProvenance const*> theProvenance;
  iEvent.getAllStableProvenance(theProvenance);
  for(auto const& provenance : theProvenance) {
    std::cout<<" Print all module/label names "<<provenance->moduleName()<<" "<<provenance->moduleLabel()
	     <<" "<<provenance->productInstanceName()<<std::endl;
  }
  
  
  if(nameProd_ == "hoCalibProducer") {
    edm::Handle<HOCalibVariableCollection> ho;
    iEvent.getByToken(tok_hovar_, ho);
    const HOCalibVariableCollection Hitho = *(ho.product());
    std::cout<<" Size of HO "<<(Hitho).size()<<std::endl;
  }
  
   if(nameProd_ == "ALCARECOMuAlZMuMu" ) {
   
     edm::Handle<HORecHitCollection> ho;
     iEvent.getByToken(tok_horeco_, ho);
     const HORecHitCollection Hitho = *(ho.product());
     std::cout<<" Size of HO "<<(Hitho).size()<<std::endl;
     edm::Handle<MuonCollection> mucand;
     iEvent.getByToken(tok_muons_, mucand);
     std::cout<<" Size of muon collection "<<mucand->size()<<std::endl;
     for(const auto & it : *(mucand.product()))  {
       TrackRef mu = it.combinedMuon();
       std::cout<<" Pt muon "<<mu->innerMomentum()<<std::endl;
     }
     
   }  
  
   if(nameProd_ != "IsoProd" && nameProd_ != "ALCARECOMuAlZMuMu" && nameProd_ != "hoCalibProducer") {
     edm::Handle<HBHERecHitCollection> hbhe;
     iEvent.getByToken(tok_hbhe_, hbhe);
     const HBHERecHitCollection Hithbhe = *(hbhe.product());
     std::cout<<" Size of HBHE "<<(Hithbhe).size()<<std::endl;


     edm::Handle<HORecHitCollection> ho;
     iEvent.getByToken(tok_ho_, ho);
     const HORecHitCollection Hitho = *(ho.product());
     std::cout<<" Size of HO "<<(Hitho).size()<<std::endl;


     edm::Handle<HFRecHitCollection> hf;
     iEvent.getByToken(tok_hf_, hf);
     const HFRecHitCollection Hithf = *(hf.product());
     std::cout<<" Size of HF "<<(Hithf).size()<<std::endl;
   }
   if(nameProd_ == "IsoProd") {
     std::cout<<" We are here "<<std::endl;
     edm::Handle<reco::TrackCollection> tracks;
     iEvent.getByToken(tok_tracks_,tracks);
 
   
     std::cout<<" Tracks size "<<(*tracks).size()<<std::endl;
     for (const auto & track : *(tracks.product())) {
       std::cout<<" P track "<<track.p()<<" eta "<<track.eta()<<" phi "<<track.phi()<<" Outer "
		<<track.outerMomentum()<<" "<<track.outerPosition()<<std::endl;
       TrackExtraRef myextra = track.extra();
       std::cout<<" Track extra "<<myextra->outerMomentum()<<" "<<myextra->outerPosition()<<std::endl;
     }  

     edm::Handle<EcalRecHitCollection> ecal;
     iEvent.getByToken(tok_ecal_,ecal);
     const EcalRecHitCollection Hitecal = *(ecal.product());
     std::cout<<" Size of Ecal "<<(Hitecal).size()<<std::endl;

     double energyECAL = 0.;
     double energyHCAL = 0.;

     for (const auto & hite : *(ecal.product())) {

//           std::cout<<" Energy ECAL "<<hite.energy()<<std::endl;
//	   " eta "<<hite.detid()<<" phi "<<hite.detid().getPosition().phi()<<std::endl;

       const GlobalPoint& posE = geo->getPosition(hite.detid());
       
       std::cout<<" Energy ECAL "<<hite.energy()<<
	 " eta "<<posE.eta()<<" phi "<<posE.phi()<<std::endl;

       energyECAL = energyECAL + hite.energy();
       
     }

     edm::Handle<HBHERecHitCollection> hbhe;
     iEvent.getByToken(tok_hbheProd_,hbhe);
     const HBHERecHitCollection Hithbhe = *(hbhe.product());
     std::cout<<" Size of HBHE "<<(Hithbhe).size()<<std::endl;

     for (const auto & hith : *(hbhe.product())) {

       GlobalPoint posH = (static_cast<const HcalGeometry*>(geo->getSubdetectorGeometry(hith.detid())))->getPosition(hith.detid());
     
       std::cout<<" Energy HCAL "<<hith.energy()
		<<" eta "<<posH.eta()<<" phi "<<posH.phi()<<std::endl;

       energyHCAL = energyHCAL + hith.energy();
	 
     }
   
     std::cout<<" Energy ECAL "<< energyECAL<<" Energy HCAL "<< energyHCAL<<std::endl;
     /*
     edm::Handle<HORecHitCollection> ho;
     iEvent.getByToken(tok_hoProd_,ho);
     const HORecHitCollection Hitho = *(ho.product());
     std::cout<<" Size of HO "<<(Hitho).size()<<std::endl;

     for (const auto & hito : *(ho.product())) {
       std::cout<<" Energy HO    "<<hito.energy()<<std::endl;
     }
     */
   }
   
   
   if(nameProd_ == "GammaJetProd" || nameProd_ == "DiJProd")  {
     std::cout<<" we are in GammaJetProd area "<<std::endl;
     edm::Handle<EcalRecHitCollection> ecal;
     iEvent.getByToken(tok_ecal_, ecal);
     std::cout<<" Size of ECAL "<<(*ecal).size()<<std::endl;

     edm::Handle<reco::CaloJetCollection> jets;
     iEvent.getByToken(tok_jets_, jets);
     std::cout<<" Jet size "<<(*jets).size()<<std::endl; 

     for (const auto & jet : *(jets.product())) {
       std::cout<<" Et jet "<<jet.et()<<" eta "<<jet.eta()<<" phi "<<jet.phi()<<std::endl;
     }  

     edm::Handle<reco::TrackCollection> tracks;
     iEvent.getByToken(tok_tracks_, tracks);
     std::cout<<" Tracks size "<<(*tracks).size()<<std::endl; 
   }
   if( nameProd_ == "GammaJetProd")  {
     edm::Handle<reco::SuperClusterCollection> eclus;
     iEvent.getByToken(tok_gamma_, eclus);
     std::cout<<" GammaClus size "<<(*eclus).size()<<std::endl;
     for (const auto & iclus : *(eclus.product())) {
       std::cout<<" Et gamma "<<iclus.energy()/cosh(iclus.eta())<<" eta "<<iclus.eta()<<" phi "<<iclus.phi()<<std::endl;
     }
   }

}
//define this as a plug-in
//DEFINE_FWK_MODULE(ProducerAnalyzer)
}
