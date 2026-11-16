// ============================================================================
//  PhysicsList.hh
// ----------------------------------------------------------------------------
//  The custom modular physics list (Session 2 target). Replaces the placeholder
//  FTFP_BERT. Inherits G4VModularPhysicsList and registers four constructors.
//
//  Why each one (Custom Physics List.md):
//    - G4EmStandardPhysics_option4  : highest-accuracy EM. Critical for correct
//      Bremsstrahlung yields in high-Z layers (Pb), which is a CORE physical
//      effect the whole Graded-Z argument depends on.
//    - G4HadronPhysicsFTFP_BERT_HP  : hadronic model + High Precision neutron
//      data. Without _HP, thermal-neutron transport & capture are fictional —
//      and the Am-Be source is half neutrons.
//    - G4HadronElasticPhysicsHP     : HP elastic scattering, i.e. the actual
//      moderation of fast neutrons by hydrogen in HDPE.
//    - G4RadioactiveDecayPhysics    : decay chains (needed once we model the
//      isotopic Am-Be source properly).
//
//  PITFALL: each RegisterPhysics() hands ownership to the framework. Keep no raw
//  pointers, never delete them.
// ============================================================================
#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
    PhysicsList();
    ~PhysicsList() override = default;

    // Optional: tighten production cuts. We override to set a sane global cut.
    void SetCuts() override;
};

#endif  // PHYSICSLIST_HH
