// -*- C++ -*-
//
// Package:    HFJetShowerShape/HFJetShowerShape
// Class:      HFJetShowerShape
// 
/**\class HFJetShowerShape HFJetShowerShape.cc HFJetShowerShape/HFJetShowerShape/plugins/HFJetShowerShape.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Laurent Thomas
//         Created:  Tue, 25 Aug 2020 09:17:42 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "DataFormats/Math/interface/deltaPhi.h"
#include "TMath.h"
#include <iostream>

//
// class declaration
//

class HFJetShowerShape : public edm::stream::EDProducer<> {
public:
  explicit HFJetShowerShape(const edm::ParameterSet&);
  ~HFJetShowerShape();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  template <typename T>
  void putInEvent(const std::string&, const edm::Handle<edm::View<reco::Jet>>&, std::vector<T>*, edm::Event&) const;
  
  edm::EDGetTokenT<edm::View<reco::Jet> > jets_token_;
  edm::EDGetTokenT<std::vector<reco::Vertex> > vertices_token_; 

  //Jet pt/eta thresholds
  double jetPtThreshold_,jetEtaThreshold_;
  //HF geometry
  double hfTowerEtaWidth_, hfTowerPhiWidth_;
  //Variables for PU subtraction
  double vertexRecoEffcy_, offsetPerPU_, jetReferenceRadius_;
  //Pt thresholds for showershape variable calculation
  double stripPtThreshold_,widthPtThreshold_;

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
HFJetShowerShape::HFJetShowerShape(const edm::ParameterSet& iConfig)
{
  jets_token_ = consumes<edm::View<reco::Jet> > (iConfig.getParameter<edm::InputTag>("TheJets"));
  vertices_token_ = consumes<std::vector<reco::Vertex> > (iConfig.getParameter<edm::InputTag>("TheVertices"));
  jetPtThreshold_= iConfig.getParameter<double>("JetPtThreshold");
  jetEtaThreshold_= iConfig.getParameter<double>("JetEtaThreshold");
  hfTowerEtaWidth_= iConfig.getParameter<double>("HFTowerEtaWidth");
  hfTowerPhiWidth_= iConfig.getParameter<double>("HFTowerPhiWidth");
  vertexRecoEffcy_= iConfig.getParameter<double>("VertexRecoEffcy");
  offsetPerPU_= iConfig.getParameter<double>("OffsetPerPU");
  jetReferenceRadius_= iConfig.getParameter<double>("JetReferenceRadius");
  stripPtThreshold_= iConfig.getParameter<double>("StripPtThreshold");
  widthPtThreshold_= iConfig.getParameter<double>("WidthPtThreshold");


  produces<edm::ValueMap<float> >("sigmaEtaEta");
  produces<edm::ValueMap<float> >("sigmaPhiPhi");
  produces<edm::ValueMap<int> >("centralEtaStripSize");
  produces<edm::ValueMap<int> >("adjacentEtaStripsSize");

}


HFJetShowerShape::~HFJetShowerShape()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HFJetShowerShape::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

      
   
   //Jets
   edm::Handle<edm::View<reco::Jet>> theJets;
   iEvent.getByToken(jets_token_, theJets);

   //Vertices
   edm::Handle<std::vector<reco::Vertex> > theVertices;
   iEvent.getByToken(vertices_token_,theVertices) ;
   int nPV = theVertices->size();
   
   //Products
   std::vector<float>*v_sigmaEtaEta  = new std::vector<float>; 
   std::vector<float>*v_sigmaPhiPhi  = new std::vector<float>; 
   std::vector<int>*v_size_CentralEtaStrip  = new std::vector<int>;
   std::vector<int>*v_size_AdjacentEtaStrips  = new std::vector<int>;

   //Et offset for HF PF candidates 
   double PUoffset= offsetPerPU_/ ( TMath::Pi() * jetReferenceRadius_*jetReferenceRadius_ )  * nPV/ vertexRecoEffcy_ * (hfTowerEtaWidth_* hfTowerPhiWidth_); 
   

   for (auto jet = theJets->begin(); jet != theJets->end(); ++jet) {
     double pt_jet = jet->pt();
     double eta_jet = jet->eta();
     
     //If central or low pt jets, fill with dummy variables
     if(pt_jet <= jetPtThreshold_ || fabs(eta_jet) <= jetEtaThreshold_ ){
       v_sigmaEtaEta->push_back(-1.);
       v_sigmaPhiPhi->push_back(-1.);
       v_size_CentralEtaStrip->push_back(0);
       v_size_AdjacentEtaStrips->push_back(0);
     }
     else{
       //First loop over PF candidates to compute some global variables needed for shower shape calculations
       double sumptPFcands = 0.;
       
       for (unsigned i = 0; i < jet->numberOfSourceCandidatePtrs(); ++i) {
	 reco::CandidatePtr pfJetConstituent = jet->sourceCandidatePtr(i);
	 const reco::Candidate* icand = pfJetConstituent.get();
	 //Only look at pdgId =1,2 (HF PF cands)
	 if(fabs(icand->pdgId() )>2) continue;
	 double pt_PUsub = icand->pt() - PUoffset;
	 if(pt_PUsub < widthPtThreshold_ ) continue;
	 sumptPFcands+=  pt_PUsub;
       }
       
       //Second loop to compute the various shower shape variables
       int size_CentralEtaStrip(0.), size_AdjacentEtaStrips(0.);
       double sigmaEtaEtaSq(0.),sigmaPhiPhiSq(0.);
       double sumweightsPFcands = 0;
       for (unsigned i = 0; i < jet->numberOfSourceCandidatePtrs(); ++i) {
	 reco::CandidatePtr pfJetConstituent = jet->sourceCandidatePtr(i);
	 const reco::Candidate* icand = pfJetConstituent.get();
	 //Only look at pdgId =1,2 (HF PF cands)
	 if(fabs(icand->pdgId() )>2) continue;
	 
	 double deta=  fabs( icand->eta()-jet->eta() );       
	 double dphi=  fabs(deltaPhi(icand->phi(),jet->phi()));
	 double pt_PUsub = icand->pt() - PUoffset;
	 
	 //This is simply the size of the central eta strip and the adjacent strips
	 if( pt_PUsub >= stripPtThreshold_ ){
	   if( dphi <= hfTowerPhiWidth_ *0.5 ) size_CentralEtaStrip ++;
	   else if( dphi <= hfTowerPhiWidth_ *1.5 ) size_AdjacentEtaStrips ++;
	 }
	 
	 //Now computing sigmaEtaEta/PhiPhi
	 if( pt_PUsub >= widthPtThreshold_ && sumptPFcands>0){
	   double weight =  pt_PUsub/sumptPFcands;
	   sigmaEtaEtaSq += deta*deta*weight;
	   sigmaPhiPhiSq += dphi*dphi*weight;
	   sumweightsPFcands+=weight;
	 }
       }
       
       v_size_CentralEtaStrip->push_back(size_CentralEtaStrip);
       v_size_AdjacentEtaStrips->push_back(size_AdjacentEtaStrips);
       
       if(sumweightsPFcands>0&&sigmaEtaEtaSq>0&&sigmaPhiPhiSq>0){
	 v_sigmaEtaEta->push_back(sqrt(sigmaEtaEtaSq/sumweightsPFcands));
	 v_sigmaPhiPhi->push_back(sqrt(sigmaPhiPhiSq/sumweightsPFcands));
       }
       else{
	 v_sigmaEtaEta->push_back(-1.);
	 v_sigmaPhiPhi->push_back(-1.);
       }
       
     }//End loop over jets
   }

   putInEvent("sigmaEtaEta", theJets, v_sigmaEtaEta, iEvent);
   putInEvent("sigmaPhiPhi", theJets, v_sigmaPhiPhi, iEvent);
   putInEvent("centralEtaStripSize", theJets, v_size_CentralEtaStrip, iEvent);
   putInEvent("adjacentEtaStripsSize", theJets, v_size_AdjacentEtaStrips, iEvent);

   
}

 /// Function to put product into event
template <typename T>
void HFJetShowerShape::putInEvent(const std::string& name,
                          const edm::Handle<edm::View<reco::Jet>>& jets,
                          std::vector<T>* product,
                          edm::Event& iEvent) const {
  auto out = std::make_unique<edm::ValueMap<T>>();
  typename edm::ValueMap<T>::Filler filler(*out);
  filler.insert(jets, product->begin(), product->end());
  filler.fill();
  iEvent.put(std::move(out), name);
  delete product;
}



// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HFJetShowerShape::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {

  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("TheJets", edm::InputTag("slimmedJets"));
  desc.add<edm::InputTag>("TheVertices", edm::InputTag("offlineSlimmedPrimaryVertices"));
  desc.add<double>("JetPtThreshold",25.);
  desc.add<double>("JetEtaThreshold",2.9);
  desc.add<double>("HFTowerEtaWidth",0.175);
  desc.add<double>("HFTowerPhiWidth",0.175);
  desc.add<double>("VertexRecoEffcy",0.7);
  desc.add<double>("OffsetPerPU",0.4);
  desc.add<double>("JetReferenceRadius",0.4);
  desc.add<double>("StripPtThreshold",10.);
  desc.add<double>("WidthPtThreshold",3.);  
  descriptions.add("jetHFShowerShapeProducer", desc);

}

//define this as a plug-in
DEFINE_FWK_MODULE(HFJetShowerShape);
