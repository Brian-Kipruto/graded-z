// ============================================================================
//  LayerSpec.hh
// ----------------------------------------------------------------------------
//  A plain-old-data struct describing ONE layer of the Graded-Z stack.
//
//  This is the single most important data structure in the geometry half of
//  the project. The whole "one binary, infinite configurations" promise
//  (Architecture Lockdown) rests on geometry being a std::vector<LayerSpec>
//  that the Messenger fills from a macro, instead of hardcoded G4Box calls.
//
//  Two kinds of layer are supported:
//    1. A pure NIST material   (e.g. "G4_POLYETHYLENE", "G4_Pb", "G4_Al")
//    2. A composite material    (matrix + metal filler at a mass fraction)
//
//  We keep BOTH descriptions in the same struct and use `isComposite` to tell
//  them apart. DetectorConstruction reads this flag to decide whether to call
//  G4NistManager::FindOrBuildMaterial (pure) or to build a custom G4Material
//  (composite). See Geometry Strategy.md and Modeling Flexibility.md.
// ============================================================================
#ifndef LAYERSPEC_HH
#define LAYERSPEC_HH

#include "G4String.hh"
#include "G4SystemOfUnits.hh"   // gives us `mm`, `cm`, etc.

struct LayerSpec {
    // --- common to every layer ---------------------------------------------
    G4double thickness = 1.0 * mm;   // FULL thickness (NOT half). DetectorConstruction
                                     // halves it when building the G4Box.

    // --- pure-material description ------------------------------------------
    G4String material = "G4_Galactic";  // NIST material name. Used when !isComposite.

    // --- composite description ---------------------------------------------
    bool      isComposite   = false;
    G4String  matrix        = "";    // e.g. "G4_POLYETHYLENE" or "G4_EPOXY"
    G4String  filler        = "";    // e.g. "G4_Pb", "G4_W", "G4_Bi"
    G4double  fillerMassFrac = 0.0;  // 0..1, fraction of total MASS that is filler

    // A human-readable label, used as the CSV column suffix / scoring name.
    // For pure layers this is just the material; for composites we synthesise
    // something like "EPOXY+Pb30". DetectorConstruction sets it.
    G4String label = "layer";
};

#endif  // LAYERSPEC_HH
