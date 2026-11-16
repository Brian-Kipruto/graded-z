// ============================================================================
//  DetectorMessenger.hh
// ----------------------------------------------------------------------------
//  THE KEYSTONE (DetectorMessenger.md). Exposes /shield/... UI commands so the
//  geometry is configured from a MACRO, not from recompiled C++. Without this,
//  the Python orchestrator cannot exist.
//
//  Command vocabulary (Architecture Lockdown interface #1):
//    /shield/clearLayers
//    /shield/addLayer <material> <thickness> <unit>
//    /shield/addCompositeLayer <matrix> <filler> <massFrac> <thickness> <unit>
//    /shield/setSourceDistance <z> <unit>
//    /shield/build
//
//  Ownership: DetectorConstruction owns this object and deletes it. This object
//  owns the G4UIcommand* it creates and deletes them in its own dtor.
// ============================================================================
#ifndef DETECTORMESSENGER_HH
#define DETECTORMESSENGER_HH

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADoubleAndUnit;

class DetectorMessenger : public G4UImessenger {
public:
    explicit DetectorMessenger(DetectorConstruction* det);
    ~DetectorMessenger() override;

    // Called by Geant4 whenever one of our registered commands is invoked.
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorConstruction* fDetector;   // not owned (it owns US)

    G4UIdirectory*            fShieldDir         = nullptr;
    G4UIcmdWithoutParameter*  fClearCmd          = nullptr;
    G4UIcommand*              fAddLayerCmd       = nullptr;  // multi-param → raw G4UIcommand
    G4UIcommand*              fAddCompositeCmd   = nullptr;
    G4UIcmdWithADoubleAndUnit* fSourceDistCmd    = nullptr;
    G4UIcmdWithoutParameter*  fBuildCmd          = nullptr;
};

#endif  // DETECTORMESSENGER_HH
