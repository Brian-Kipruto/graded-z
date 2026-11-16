// ============================================================================
//  SteppingAction.cc
// ----------------------------------------------------------------------------
//  The classification logic. Read this slowly — it is the conceptual heart of
//  "how we gather the ML features".
// ============================================================================
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

// ----------------------------------------------------------------------------
SteppingAction::SteppingAction(EventAction* evt, const DetectorConstruction* det)
    : G4UserSteppingAction(), fEvent(evt), fDetector(det) {}

// ----------------------------------------------------------------------------
void SteppingAction::UserSteppingAction(const G4Step* step) {
    EventData& data = fEvent->Data();

    const G4Track* track = step->GetTrack();
    const G4StepPoint* pre  = step->GetPreStepPoint();
    const G4StepPoint* post = step->GetPostStepPoint();

    // ---- (1) Record incident energy of primaries on their FIRST step -------
    // A primary has TrackID 1 (or, with multiple primaries per event, parentID
    // 0). GetCurrentStepNumber()==1 means this is its first step → its energy
    // here is the energy it entered the world with.
    if (track->GetParentID() == 0 && track->GetCurrentStepNumber() == 1) {
        data.incidentEnergy += pre->GetKineticEnergy();
    }

    // ---- (2) Detect crossing of the stack BACK FACE in the +z direction ----
    const G4double backZ = fDetector->GetStackBackZ();
    const G4double zPre  = pre->GetPosition().z();
    const G4double zPost = post->GetPosition().z();

    // Crossed the plane this step if pre is before and post is at/after it,
    // and the particle is moving forward (+z momentum).
    const bool crossedBackFace =
        (zPre < backZ) && (zPost >= backZ) &&
        (post->GetMomentumDirection().z() > 0.0);

    if (!crossedBackFace) return;   // nothing else to do this step

    // We are at the moment a particle exits the back of the shield stack.
    const G4ParticleDefinition* pdef = track->GetParticleDefinition();
    const G4String pname = pdef->GetParticleName();
    const G4double ke    = post->GetKineticEnergy();

    // Total transmitted tally (for mean transmitted energy later).
    data.transmittedEnergy += ke;
    data.nTransmittedTotal += 1;

    const bool isPrimary = (track->GetParentID() == 0);

    // What process created this track? (nullptr for primaries.)
    const G4VProcess* creator = track->GetCreatorProcess();
    const G4String cproc = creator ? creator->GetProcessName() : G4String("primary");

    // ---- (3) Bin it -------------------------------------------------------
    if (pname == "gamma") {
        if (cproc == "nCapture") {
            data.nCaptureGammas += 1;     // e.g. 2.2 MeV H-capture gamma
        } else {
            data.nSecondaryPhotons += 1;  // brems, fluorescence, inelastic gammas
        }
    } else if (pname == "e-") {
        if (isPrimary) data.nPrimaryTransmitted += 1;   // a survived primary e-
        else           data.nSecondaryElectrons += 1;
    } else if (pname == "neutron") {
        data.nNeutronsTransmitted += 1;
        if (isPrimary) data.nPrimaryTransmitted += 1;   // survived primary neutron
    } else {
        // Other primaries (e.g. primary gamma from Am-Be) that survive.
        if (isPrimary) data.nPrimaryTransmitted += 1;
    }
}
