// ============================================================================
//  ActionInitialization.cc
// ============================================================================
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

// ----------------------------------------------------------------------------
ActionInitialization::ActionInitialization(const DetectorConstruction* det)
    : G4VUserActionInitialization(), fDetector(det) {}

// ----------------------------------------------------------------------------
void ActionInitialization::BuildForMaster() const {
    // The master thread only needs RunAction (to merge/close the output file).
    SetUserAction(new RunAction(fDetector));
}

// ----------------------------------------------------------------------------
void ActionInitialization::Build() const {
    // Primary generator.
    SetUserAction(new PrimaryGeneratorAction());

    // Run action (opens/closes CSV, owns schema).
    SetUserAction(new RunAction(fDetector));

    // Event action (assembles + writes one row per event). Stepping action
    // needs a pointer to it so it can fill the shared EventData.
    auto* eventAction = new EventAction(fDetector);
    SetUserAction(eventAction);

    // Stepping action (particle classification at the back face).
    SetUserAction(new SteppingAction(eventAction, fDetector));
}
