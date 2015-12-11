#include "GeantSvc.h"
// Gaudi
#include "GaudiKernel/IToolSvc.h"
// FCCSW
#include "DetDesInterfaces/IGeoSvc.h"

// Geant
#include "G4Event.hh"
#include "G4VModularPhysicsList.hh"

DECLARE_SERVICE_FACTORY(GeantSvc)

GeantSvc::GeantSvc(const std::string& aName, ISvcLocator* aSL): base_class(aName, aSL) {
  declareProperty("config", m_geantConfigTool);
  declarePrivateTool(m_geantConfigTool);
}

GeantSvc::~GeantSvc(){}

StatusCode GeantSvc::initialize(){
  // Initialize necessary Gaudi components
  if (Service::initialize().isFailure()){
    error()<<"Unable to initialize Service()"<<endmsg;
    return StatusCode::FAILURE;
  }
  m_toolSvc = service("ToolSvc");
  if (!m_toolSvc) {
    error()<<"Unable to locate Tool Service"<<endmsg;
    return StatusCode::FAILURE;
  }
  m_geoSvc = service ("GeoSvc");
  if (!m_geoSvc) {
    error()<<"Unable to locate Geometry Service"<<endmsg;
    return StatusCode::FAILURE;
  }
  if (!m_geantConfigTool.retrieve()) {
    error()<<"Unable to locate Geant configuration"<<endmsg;
    return StatusCode::FAILURE;
  }
  // Initialize Geant run manager
  // Initialization - Geant part
  // Load physics list, deleted in ~G4RunManager()
  m_runManager.SetUserInitialization(m_geantConfigTool->getPhysicsList());

  // Take geometry (from DD4Hep), deleted in ~G4RunManager()
  m_runManager.SetUserInitialization(m_geoSvc->getGeant4Geo());

  m_runManager.Initialize();
  // Attach user actions
  m_runManager.SetUserInitialization(m_geantConfigTool->getActionInitialization());

  m_geantConfigTool->getOtherSettings();

  if( !m_runManager.start()) {
    error() << "Unable to initialize GEANT correctly." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
StatusCode GeantSvc::processEvent(G4Event* aEvent) {
  bool status = m_runManager.processEvent( aEvent );
  if ( !status ) {
     error() << "Unable to process event in Geant" << endmsg;
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
StatusCode GeantSvc::retrieveEvent(const G4Event*& aEvent) {
  return m_runManager.retrieveEvent(aEvent);
}

StatusCode GeantSvc::terminateEvent() {
  m_runManager.terminateEvent();
  return StatusCode::SUCCESS;
}
StatusCode GeantSvc::finalize() {
  m_runManager.finalize();
  return StatusCode::SUCCESS;
}