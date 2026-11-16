// ============================================================================
//  DetectorConstruction.cc
// ============================================================================
#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include <sstream>
#include <cmath>

// ----------------------------------------------------------------------------
DetectorConstruction::DetectorConstruction() {
    // Create the messenger so the macro vocabulary is live from the start.
    // (We own it; deleted in dtor.)
    fMessenger = new DetectorMessenger(this);

    // Seed a sensible DEFAULT stack so `./graded-z` with no macro still draws
    // the proposal's PE/Pb/Al graded-Z (Figure 5). The sweep overwrites this.
    AddLayer("G4_POLYETHYLENE", 30.0 * mm);
    AddLayer("G4_Pb",           10.0 * mm);
    AddLayer("G4_Al",           10.0 * mm);
}

// ----------------------------------------------------------------------------
DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;   // WE own this one. Volumes are framework-owned, don't touch.
}

// ----------------------------------------------------------------------------
void DetectorConstruction::ClearLayers() {
    fLayers.clear();
}

// ----------------------------------------------------------------------------
void DetectorConstruction::AddLayer(const G4String& material, G4double thickness) {
    LayerSpec s;
    s.material    = material;
    s.thickness   = thickness;
    s.isComposite = false;
    // Strip the "G4_" prefix for a tidy label, e.g. "G4_Pb" -> "Pb".
    s.label = (material.rfind("G4_", 0) == 0) ? G4String(material.substr(3)) : material;
    fLayers.push_back(s);
}

// ----------------------------------------------------------------------------
void DetectorConstruction::AddCompositeLayer(const G4String& matrix,
                                             const G4String& filler,
                                             G4double fillerMassFrac,
                                             G4double thickness) {
    LayerSpec s;
    s.isComposite    = true;
    s.matrix         = matrix;
    s.filler         = filler;
    s.fillerMassFrac = fillerMassFrac;
    s.thickness      = thickness;

    auto strip = [](const G4String& m) {
        return (m.rfind("G4_", 0) == 0) ? G4String(m.substr(3)) : m;
    };
    std::ostringstream lbl;
    lbl << strip(matrix) << "+" << strip(filler)
        << static_cast<int>(std::lround(fillerMassFrac * 100));
    s.label = lbl.str();   // e.g. "EPOXY+Pb30"
    fLayers.push_back(s);
}

// ----------------------------------------------------------------------------
//  Composite material: matrix + metal filler at a given MASS fraction.
//  IMPORTANT (Modeling Flexibility.md): the effective density of a real
//  particle-filled polymer is NOT the mass-weighted average of constituent
//  densities. Real composites have voids/packing effects. For a first pass we
//  use the rule of mixtures on SPECIFIC VOLUME, which is the physically correct
//  closed-form for an ideal (void-free) mixture:
//
//      1 / rho_mix = w_matrix / rho_matrix + w_filler / rho_filler
//
//  This is exact for a void-free blend and is the honest default. If lab
//  measurements of real composite density arrive later, override via macro.
// ----------------------------------------------------------------------------
G4Material* DetectorConstruction::BuildCompositeMaterial(const LayerSpec& spec) {
    auto* nist = G4NistManager::Instance();
    G4Material* matrixMat = nist->FindOrBuildMaterial(spec.matrix);
    G4Material* fillerMat = nist->FindOrBuildMaterial(spec.filler);

    if (!matrixMat || !fillerMat) {
        G4Exception("DetectorConstruction::BuildCompositeMaterial", "BadMat",
                    FatalException, "Unknown matrix or filler material name.");
        return nullptr;
    }

    const G4double wF = spec.fillerMassFrac;     // filler mass fraction
    const G4double wM = 1.0 - wF;                // matrix mass fraction
    const G4double rhoM = matrixMat->GetDensity();
    const G4double rhoF = fillerMat->GetDensity();

    // Void-free rule of mixtures on specific volume:
    const G4double invRho = wM / rhoM + wF / rhoF;
    const G4double rhoMix = 1.0 / invRho;

    // Build a uniquely-named material so repeated builds don't collide.
    std::ostringstream name;
    name << "COMP_" << spec.label;
    // If it already exists (geometry rebuilt with same config), reuse it.
    if (auto* existing = G4Material::GetMaterial(name.str(), false)) {
        return existing;
    }

    auto* mix = new G4Material(name.str(), rhoMix, /*nComponents=*/2);
    mix->AddMaterial(matrixMat, wM);
    mix->AddMaterial(fillerMat, wF);
    return mix;
}

// ----------------------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto* nist = G4NistManager::Instance();

    // Reset cached LV pointers (Construct may run many times via rebuild).
    fLayerLVs.clear();
    fDetectorLV = nullptr;

    // ---- World: a box of air (so room-scatter/albedo are crudely present) --
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");

    // Size the world generously around the stack + source distance.
    G4double stackThickness = 0.0;
    for (const auto& L : fLayers) stackThickness += L.thickness;
    const G4double halfXY = 0.5 * m;
    fWorldHalfZ = fSourceDistance + stackThickness + 0.5 * m;  // margin behind

    auto* worldBox = new G4Box("World", halfXY, halfXY, fWorldHalfZ);
    auto* worldLV  = new G4LogicalVolume(worldBox, air, "World");
    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());
    auto* worldPV  = new G4PVPlacement(
        nullptr, G4ThreeVector(), worldLV, "World",
        nullptr, false, 0, /*checkOverlaps=*/true);   // rule 11: always true in dev

    // ---- The Graded-Z stack -----------------------------------------------
    // The stack FRONT face sits at z = 0. Layers extend toward +z. The source
    // is at z = -fSourceDistance, firing +z. So the beam hits layer 0 first.
    G4double cumulativeZ = 0.0;   // running front-edge position

    // A simple recurring palette for visualization.
    auto colourFor = [](const G4String& label) -> G4Colour {
        if (label.find("POLY") != G4String::npos) return G4Colour(1, 1, 1);   // white
        if (label.find("Pb")   != G4String::npos) return G4Colour(0.6, 0.4, 0.2); // brown
        if (label.find("Al")   != G4String::npos) return G4Colour(0, 1, 1);   // cyan
        return G4Colour(0.8, 0.8, 0.2);                                        // default
    };

    for (std::size_t i = 0; i < fLayers.size(); ++i) {
        const LayerSpec& spec = fLayers[i];

        // Resolve the material.
        G4Material* mat = spec.isComposite
            ? BuildCompositeMaterial(spec)
            : nist->FindOrBuildMaterial(spec.material);
        if (!mat) {
            G4Exception("DetectorConstruction::Construct", "BadMat",
                        FatalException, ("Unknown material: " + spec.material).c_str());
        }

        // G4Box takes HALF extents. Halve ONCE. (Geometry Strategy landmine.)
        const G4double halfThick = 0.5 * spec.thickness;

        std::ostringstream solidName; solidName << "Layer" << i << "_" << spec.label;
        auto* box = new G4Box(solidName.str(), halfXY, halfXY, halfThick);
        auto* lv  = new G4LogicalVolume(box, mat, solidName.str() + "_LV");

        // Centre of layer i = front edge + half thickness.
        const G4double posZ = cumulativeZ + halfThick;
        new G4PVPlacement(
            nullptr, G4ThreeVector(0, 0, posZ), lv, solidName.str(),
            worldLV, false, static_cast<G4int>(i), /*checkOverlaps=*/true);

        // Visualization.
        auto* vis = new G4VisAttributes(colourFor(spec.label));
        vis->SetForceSolid(true);
        lv->SetVisAttributes(vis);

        fLayerLVs.push_back(lv);     // remember it for scoring
        cumulativeZ += spec.thickness;
    }

    fStackBackZ = cumulativeZ;   // z of the back face of the last layer

    // ---- "Detector" slab right behind the stack ----------------------------
    // A thin layer of NaI is a stand-in for the NaI(Tl) gamma spectrometer.
    // We score energy deposit here as the "transmitted dose" proxy.
    G4Material* nai = nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");
    const G4double detThick   = 5.0 * mm;
    const G4double detHalf     = 0.5 * detThick;
    const G4double detGap      = 1.0 * mm;   // tiny air gap behind stack
    const G4double detZ        = fStackBackZ + detGap + detHalf;

    auto* detBox = new G4Box("Detector", halfXY, halfXY, detHalf);
    fDetectorLV  = new G4LogicalVolume(detBox, nai, "Detector_LV");
    new G4PVPlacement(
        nullptr, G4ThreeVector(0, 0, detZ), fDetectorLV, "Detector",
        worldLV, false, 0, /*checkOverlaps=*/true);

    auto* detVis = new G4VisAttributes(G4Colour(1, 0, 0));  // red
    detVis->SetForceSolid(true);
    fDetectorLV->SetVisAttributes(detVis);

    return worldPV;
}

// ----------------------------------------------------------------------------
//  ConstructSDandField: attach a G4MultiFunctionalDetector with a
//  G4PSEnergyDeposit scorer to EACH layer and to the detector slab.
//
//  Why MFD + primitive scorer (Scoring Strategy.md choice #1):
//    - It's the framework-blessed, leak-proof way to tally energy deposit.
//    - One named hits-map per volume; EventAction reads them by collection ID.
//  Particle-type classification (choice #2) is done separately in
//  SteppingAction, because primitive scorers can't see creator processes.
// ----------------------------------------------------------------------------
void DetectorConstruction::ConstructSDandField() {
    auto* sdMgr = G4SDManager::GetSDMpointer();

    // One MFD per layer, named "Layer<i>_MFD", with an "Edep" scorer.
    for (std::size_t i = 0; i < fLayerLVs.size(); ++i) {
        std::ostringstream mfdName; mfdName << "Layer" << i << "_MFD";
        auto* mfd = new G4MultiFunctionalDetector(mfdName.str());
        sdMgr->AddNewDetector(mfd);

        auto* edep = new G4PSEnergyDeposit("Edep");   // scorer name
        mfd->RegisterPrimitive(edep);

        fLayerLVs[i]->SetSensitiveDetector(mfd);
        // Collection name EventAction queries is "Layer<i>_MFD/Edep".
    }

    // The detector slab.
    if (fDetectorLV) {
        auto* detMFD = new G4MultiFunctionalDetector("Detector_MFD");
        sdMgr->AddNewDetector(detMFD);
        auto* detEdep = new G4PSEnergyDeposit("Edep");
        detMFD->RegisterPrimitive(detEdep);
        fDetectorLV->SetSensitiveDetector(detMFD);
    }
}

// ----------------------------------------------------------------------------
void DetectorConstruction::RebuildGeometry() {
    // Tell the run manager the geometry changed; it re-invokes Construct() and
    // ConstructSDandField() on the next BeamOn. This is the heart of the
    // "one binary, infinite configs" promise.
    G4RunManager::GetRunManager()->ReinitializeGeometry();
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
}
