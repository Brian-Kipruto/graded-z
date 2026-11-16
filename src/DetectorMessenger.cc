// ============================================================================
//  DetectorMessenger.cc
// ============================================================================
#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UnitsTable.hh"
#include "G4Tokenizer.hh"
#include "G4SystemOfUnits.hh"

// ----------------------------------------------------------------------------
DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
    : G4UImessenger(), fDetector(det) {

    // --- directory ---------------------------------------------------------
    fShieldDir = new G4UIdirectory("/shield/");
    fShieldDir->SetGuidance("Graded-Z shield configuration commands.");

    // --- /shield/clearLayers ----------------------------------------------
    fClearCmd = new G4UIcmdWithoutParameter("/shield/clearLayers", this);
    fClearCmd->SetGuidance("Remove all layers from the stack.");
    fClearCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // --- /shield/addLayer <material> <thickness> <unit> --------------------
    // Multiple heterogeneous parameters → use raw G4UIcommand with G4UIparameter.
    fAddLayerCmd = new G4UIcommand("/shield/addLayer", this);
    fAddLayerCmd->SetGuidance("Add a pure NIST-material layer.");
    fAddLayerCmd->SetGuidance("  Usage: /shield/addLayer <material> <thickness> <unit>");
    {
        auto* pMat = new G4UIparameter("material", 's', false);
        pMat->SetGuidance("NIST material name, e.g. G4_POLYETHYLENE, G4_Pb, G4_Al");
        fAddLayerCmd->SetParameter(pMat);

        auto* pThick = new G4UIparameter("thickness", 'd', false);
        pThick->SetGuidance("Layer thickness value.");
        fAddLayerCmd->SetParameter(pThick);

        auto* pUnit = new G4UIparameter("unit", 's', false);
        pUnit->SetGuidance("Length unit, e.g. mm, cm.");
        fAddLayerCmd->SetParameter(pUnit);
    }
    fAddLayerCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // --- /shield/addCompositeLayer <matrix> <filler> <frac> <thick> <unit> -
    fAddCompositeCmd = new G4UIcommand("/shield/addCompositeLayer", this);
    fAddCompositeCmd->SetGuidance("Add a composite layer: matrix + metal filler.");
    fAddCompositeCmd->SetGuidance("  Usage: /shield/addCompositeLayer <matrix> <filler> <massFrac> <thickness> <unit>");
    {
        auto* pMatrix = new G4UIparameter("matrix", 's', false);
        pMatrix->SetGuidance("Matrix material, e.g. G4_POLYETHYLENE, G4_EPOXY");
        fAddCompositeCmd->SetParameter(pMatrix);

        auto* pFiller = new G4UIparameter("filler", 's', false);
        pFiller->SetGuidance("Filler material, e.g. G4_Pb, G4_W, G4_Bi");
        fAddCompositeCmd->SetParameter(pFiller);

        auto* pFrac = new G4UIparameter("massFrac", 'd', false);
        pFrac->SetGuidance("Filler mass fraction (0..1).");
        fAddCompositeCmd->SetParameter(pFrac);

        auto* pThick = new G4UIparameter("thickness", 'd', false);
        pThick->SetGuidance("Layer thickness value.");
        fAddCompositeCmd->SetParameter(pThick);

        auto* pUnit = new G4UIparameter("unit", 's', false);
        pUnit->SetGuidance("Length unit, e.g. mm, cm.");
        fAddCompositeCmd->SetParameter(pUnit);
    }
    fAddCompositeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // --- /shield/setSourceDistance <z> <unit> ------------------------------
    fSourceDistCmd = new G4UIcmdWithADoubleAndUnit("/shield/setSourceDistance", this);
    fSourceDistCmd->SetGuidance("Distance of the source upstream of the stack front.");
    fSourceDistCmd->SetParameterName("z", false);
    fSourceDistCmd->SetUnitCategory("Length");
    fSourceDistCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // --- /shield/build -----------------------------------------------------
    fBuildCmd = new G4UIcmdWithoutParameter("/shield/build", this);
    fBuildCmd->SetGuidance("Rebuild geometry from the current layer list.");
    fBuildCmd->AvailableForStates(G4State_Idle);   // only after init
}

// ----------------------------------------------------------------------------
DetectorMessenger::~DetectorMessenger() {
    delete fClearCmd;
    delete fAddLayerCmd;
    delete fAddCompositeCmd;
    delete fSourceDistCmd;
    delete fBuildCmd;
    delete fShieldDir;
}

// ----------------------------------------------------------------------------
void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fClearCmd) {
        fDetector->ClearLayers();

    } else if (command == fAddLayerCmd) {
        // Tokenize "<material> <thickness> <unit>".
        G4Tokenizer next(newValue);
        G4String material = next();
        G4String thickStr = next();
        G4String unitStr  = next();
        G4double thickness = G4UIcommand::ConvertToDouble(thickStr)
                           * G4UnitDefinition::GetValueOf(unitStr);
        fDetector->AddLayer(material, thickness);

    } else if (command == fAddCompositeCmd) {
        G4Tokenizer next(newValue);
        G4String matrix = next();
        G4String filler = next();
        G4String fracStr= next();
        G4String thickStr=next();
        G4String unitStr= next();
        G4double frac     = G4UIcommand::ConvertToDouble(fracStr);
        G4double thickness= G4UIcommand::ConvertToDouble(thickStr)
                          * G4UnitDefinition::GetValueOf(unitStr);
        fDetector->AddCompositeLayer(matrix, filler, frac, thickness);

    } else if (command == fSourceDistCmd) {
        fDetector->SetSourceDistance(fSourceDistCmd->GetNewDoubleValue(newValue));

    } else if (command == fBuildCmd) {
        fDetector->RebuildGeometry();
    }
}
