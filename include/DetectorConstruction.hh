// ============================================================================
//  DetectorConstruction.hh
// ----------------------------------------------------------------------------
//  Builds the world, the Graded-Z stack (one G4Box per LayerSpec), and a thin
//  "detector" scoring slab behind the stack.
//
//  Key idea vs. Session 1: geometry is no longer hardcoded. It is rebuilt from
//  fLayers (a std::vector<LayerSpec>) every time the Messenger asks. That is
//  what makes the Python sweep possible without recompiling.
//
//  Ownership notes (Geant4 landmine):
//    - Logical/physical volumes: once you `new` them and register, Geant4 owns
//      them. Do NOT delete.
//    - fMessenger: WE own it. We `new` it in the ctor, we `delete` it in dtor.
//    - The sensitive detectors / scorers: registered with G4SDManager, framework
//      owned after registration.
// ============================================================================
#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "LayerSpec.hh"

#include <vector>

// Forward declarations keep the header lean (workflow rule 9).
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class DetectorMessenger;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    // Called by the run manager. Builds and returns the world physical volume.
    G4VPhysicalVolume* Construct() override;

    // Called (once) after Construct, to attach sensitive detectors / scorers.
    // Splitting this out is the Geant4-blessed place to register MFDs.
    void ConstructSDandField() override;

    // ---- API the Messenger drives (the macro vocabulary) -------------------
    void ClearLayers();
    void AddLayer(const G4String& material, G4double thickness);
    void AddCompositeLayer(const G4String& matrix, const G4String& filler,
                           G4double fillerMassFrac, G4double thickness);
    void SetSourceDistance(G4double z) { fSourceDistance = z; }

    // After mutating fLayers, the Messenger calls this; it triggers
    // G4RunManager::ReinitializeGeometry() so Construct() runs again.
    void RebuildGeometry();

    // ---- accessors used by other classes -----------------------------------
    G4double GetSourceDistance()       const { return fSourceDistance; }
    std::size_t GetNumberOfLayers()    const { return fLayers.size(); }
    const std::vector<LayerSpec>& GetLayers() const { return fLayers; }
    G4double GetStackBackZ()           const { return fStackBackZ; }
    G4double GetWorldHalfZ()           const { return fWorldHalfZ; }

private:
    // Helper: turn a composite LayerSpec into a real G4Material.
    G4Material* BuildCompositeMaterial(const LayerSpec& spec);

    std::vector<LayerSpec> fLayers;          // the configurable stack
    DetectorMessenger*     fMessenger = nullptr;

    G4double fSourceDistance = 30.0 * cm;    // distance of source upstream of stack front
    G4double fWorldHalfZ     = 1.0  * m;     // half-extent of world along z
    G4double fStackBackZ     = 0.0;          // z of the back face of the last layer

    // Cached pointers to logical volumes that become scoring volumes.
    // We keep them so ConstructSDandField can attach scorers per layer.
    std::vector<G4LogicalVolume*> fLayerLVs;
    G4LogicalVolume* fDetectorLV = nullptr;
};

#endif  // DETECTORCONSTRUCTION_HH
