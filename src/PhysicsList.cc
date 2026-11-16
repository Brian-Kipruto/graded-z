// ============================================================================
//  PhysicsList.cc
// ============================================================================
#include "PhysicsList.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4DecayPhysics.hh"          // ordinary particle decays (pi, mu, ...)
#include "G4EmExtraPhysics.hh"        // gamma-nuclear, e+e- → hadrons, etc.
#include "G4StoppingPhysics.hh"       // capture of stopping hadrons
#include "G4IonPhysics.hh"            // light/heavy ion transport

#include "G4SystemOfUnits.hh"

// ----------------------------------------------------------------------------
PhysicsList::PhysicsList() : G4VModularPhysicsList() {
    // The four headline constructors from the proposal §3.2.2.2:
    RegisterPhysics(new G4EmStandardPhysics_option4());   // precise EM (Brems!)
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());   // hadronic + HP neutrons
    RegisterPhysics(new G4HadronElasticPhysicsHP());      // HP elastic (moderation)
    RegisterPhysics(new G4RadioactiveDecayPhysics());     // decay chains

    // Supporting constructors that the headline four assume are present.
    // Leaving these out causes "process X needs Y" warnings or silently-missing
    // channels. They are cheap and standard in any robust list.
    RegisterPhysics(new G4DecayPhysics());                // ordinary decays
    RegisterPhysics(new G4EmExtraPhysics());              // gamma/lepto-nuclear
    RegisterPhysics(new G4StoppingPhysics());             // hadron capture at rest
    RegisterPhysics(new G4IonPhysics());                  // recoil ions

    // The default verbose level prints the constructor banner so you can VERIFY
    // (Session 2 acceptance test) the custom physics is actually loaded.
    SetVerboseLevel(1);
}

// ----------------------------------------------------------------------------
void PhysicsList::SetCuts() {
    // A 0.7 mm global production cut is the Geant4 default and fine for thin
    // shields. Per-region tighter cuts are an Open Question deferred to when we
    // care about energy-deposit resolution inside individual layers.
    SetDefaultCutValue(0.7 * mm);
    G4VModularPhysicsList::SetCuts();   // applies the cut to all particles
}
