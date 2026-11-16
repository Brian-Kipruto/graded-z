// ============================================================================
//  PrimaryGeneratorAction.hh
// ----------------------------------------------------------------------------
//  Fires primaries into the stack. We use G4GeneralParticleSource (GPS) rather
//  than G4ParticleGun because GPS lets us define a full ENERGY SPECTRUM and
//  angular distribution from the macro — exactly what an Am-Be source needs.
//
//  The Am-Be source is a MIXED field:
//    - fast neutrons : continuous spectrum ~0-11 MeV, broad peak near 4-5 MeV
//    - gammas        : a 4.4 MeV line (C-12 de-excitation)
//
//  GPS supports multiple "sources" with relative intensities, so we configure
//  two: a neutron source with a histogram/continuous spectrum, and a gamma
//  source at 4.4 MeV. All of that lives in macros/source_ambe.mac so the
//  spectrum can be tuned WITHOUT recompiling (Architecture Lockdown).
//
//  This C++ class is deliberately thin: it just owns the GPS and forwards
//  GeneratePrimaries. The physics of the source is data, set by macro.
// ============================================================================
#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"

class G4GeneralParticleSource;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;   // WE own the GPS → delete it here.

    void GeneratePrimaries(G4Event* event) override;

    G4GeneralParticleSource* GetSource() const { return fGPS; }

private:
    G4GeneralParticleSource* fGPS = nullptr;
};

#endif  // PRIMARYGENERATORACTION_HH
