// ============================================================================
//  SteppingAction.hh
// ----------------------------------------------------------------------------
//  Called once per simulation STEP (the finest granularity Geant4 exposes).
//  This is where particle CLASSIFICATION happens — the half of scoring that
//  primitive scorers cannot do, because here we can inspect:
//
//      track->GetCreatorProcess()->GetProcessName()   // "eBrem", "nCapture"...
//      track->GetParticleDefinition()                 // gamma / e- / neutron...
//      track->GetTrackID()                            // ==1 means primary
//      step->GetPreStepPoint()/GetPostStepPoint()     // geometry boundary cross
//
//  We detect a particle crossing the stack BACK FACE (z = stackBackZ) in the
//  +z direction and bin it into the EventData counters. We also record incident
//  energy by catching primaries at their first step.
// ============================================================================
#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;
class DetectorConstruction;
class G4Step;

class SteppingAction : public G4UserSteppingAction {
public:
    SteppingAction(EventAction* evt, const DetectorConstruction* det);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step* step) override;

private:
    EventAction*               fEvent;     // not owned
    const DetectorConstruction* fDetector; // not owned, for stackBackZ
};

#endif  // STEPPINGACTION_HH
