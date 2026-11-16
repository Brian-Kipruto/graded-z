// ============================================================================
//  PrimaryGeneratorAction.cc
// ============================================================================
#include "PrimaryGeneratorAction.hh"

#include "G4GeneralParticleSource.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

// ----------------------------------------------------------------------------
PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction() {
    // GPS reads almost all of its configuration from /gps/... macro commands.
    // We give it a harmless default (a single neutron) so that even with no
    // source macro the binary runs. The real Am-Be setup is in
    // macros/source_ambe.mac, applied before /run/beamOn.
    fGPS = new G4GeneralParticleSource();

    auto* nDef = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
    fGPS->SetParticleDefinition(nDef);
    fGPS->GetCurrentSource()->GetEneDist()->SetMonoEnergy(4.5 * MeV);
}

// ----------------------------------------------------------------------------
PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fGPS;   // WE own it.
}

// ----------------------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    // GPS does all the work: picks which of its configured sources fires this
    // event (by relative intensity), samples energy/position/direction, and
    // creates the primary vertex.
    fGPS->GeneratePrimaryVertex(event);
}
