#include "G4SaveTrackerHits.h"

// Geant4
#include "G4VModularPhysicsList.hh"
#include "FTFP_BERT.hh"
#include "G4Event.hh"

// albers
#include "datamodel/TrackClusterCollection.h"
#include "datamodel/TrackHitCollection.h"
#include "datamodel/TrackClusterHitsAssociationCollection.h"

// DD4hep
#include "DDG4/Geant4Hits.h"

DECLARE_TOOL_FACTORY(G4SaveTrackerHits)

G4SaveTrackerHits::G4SaveTrackerHits(const std::string& aType, const std::string& aName, const IInterface* aParent) :
GaudiTool(aType, aName, aParent) {
  declareInterface<IG4SaveOutputTool>(this);
  declareOutput("trackClusters", m_trackClusters);
  declareOutput("trackHits", m_trackHits);
  declareOutput("trackHitsClusters", m_trackHitsClusters);
  declareProperty("DataInputs", inputDataObjects());
  declareProperty("DataOutputs", outputDataObjects());
}

G4SaveTrackerHits::~G4SaveTrackerHits() {}

StatusCode G4SaveTrackerHits::initialize() {
  return GaudiTool::initialize();
}

StatusCode G4SaveTrackerHits::finalize() {
  return GaudiTool::finalize();
}

StatusCode G4SaveTrackerHits::saveOutput(const G4Event& aEvent) {
  G4HCofThisEvent* collections = aEvent.GetHCofThisEvent();
  G4VHitsCollection* collect;
  DD4hep::Simulation::Geant4TrackerHit* hit;
  if(collections != nullptr) {
    TrackClusterCollection* edmClusters = new TrackClusterCollection();
    TrackHitCollection* edmHits = new TrackHitCollection();
    TrackClusterHitsAssociationCollection* edmAssociations = new TrackClusterHitsAssociationCollection();
    for (int iter_coll=0; iter_coll<collections->GetNumberOfCollections(); iter_coll++) {
      collect = collections->GetHC(iter_coll);
      if (collect->GetName().find("Tracker") != std::string::npos) {
        int n_hit = collect->GetSize();
        info() << "\t" << n_hit<< " hits are stored in a tracker collection #"<<iter_coll<<": "<<collect->GetName()<<endmsg;
        for(auto iter_hit=0; iter_hit<n_hit; iter_hit++ ) {
          hit = dynamic_cast<DD4hep::Simulation::Geant4TrackerHit*>(collect->GetHit(iter_hit));
          TrackHitHandle edmHit = edmHits->create();
          TrackClusterHandle edmCluster = edmClusters->create();
          BareHit& edmHitCore = edmHit.mod().Core;
          BareCluster& edmClusterCore = edmCluster.mod().Core;
          edmHitCore.Cellid = hit->cellID;
          edmHitCore.Energy = hit->energyDeposit;
          edmHitCore.Time = hit->truth.time;
          edmClusterCore.position.X = hit->position.x();
          edmClusterCore.position.Y = hit->position.y();
          edmClusterCore.position.Z = hit->position.z();
          edmClusterCore.Energy = hit->energyDeposit;
          edmClusterCore.Time = hit->truth.time;
          TrackClusterHitsAssociationHandle edmAssociation = edmAssociations->create();
          edmAssociation.mod().Cluster = edmCluster;
          edmAssociation.mod().Hit = edmHit;
        }
      }
    }
    m_trackClusters.put(edmClusters);
    m_trackHits.put(edmHits);
    m_trackHitsClusters.put(edmAssociations);
  }
  return StatusCode::SUCCESS;
}