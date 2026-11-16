// ============================================================================
//  main.cc
// ----------------------------------------------------------------------------
//  Entry point. Two modes:
//    - no args         → interactive (opens the Qt/OpenGL UI + vis.mac)
//    - one arg (a .mac)→ batch (runs the macro, no UI). This is what the
//                        Python orchestrator invokes for each config.
//
//  Lifecycle order (do not reorder):
//    1. Construct run manager (SerialOnly).
//    2. Set the three mandatory initializations:
//         DetectorConstruction, PhysicsList, ActionInitialization.
//    3. Initialize visualization + UI manager.
//    4. Either execute the macro (batch) or start the interactive session.
// ============================================================================
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4SystemOfUnits.hh"

int main(int argc, char** argv) {

    // --- detect mode --------------------------------------------------------
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);   // interactive
    }

    // --- run manager (serial; the project is SerialOnly by decision) --------
    auto* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

    // --- mandatory user initializations ------------------------------------
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);
    runManager->SetUserInitialization(new PhysicsList());            // ← custom, not FTFP_BERT
    runManager->SetUserInitialization(new ActionInitialization(detector));

    // --- visualization ------------------------------------------------------
    auto* visManager = new G4VisExecutive();
    visManager->Initialize();

    auto* uiManager = G4UImanager::GetUIpointer();

    if (ui) {
        // Interactive: load default vis + default Am-Be source, then hand over.
        uiManager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    } else {
        // Batch: execute the macro the orchestrator passed.
        G4String command  = "/control/execute ";
        G4String fileName = argv[1];
        uiManager->ApplyCommand(command + fileName);
    }

    // --- cleanup ------------------------------------------------------------
    // Delete what WE own. Run manager deletes the user-initialization objects
    // (detector, physics, actions) for us, so we must NOT delete those.
    delete visManager;
    delete runManager;
    return 0;
}
