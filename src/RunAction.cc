// ============================================================================
//  RunAction.cc
// ============================================================================
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <sstream>

// ----------------------------------------------------------------------------
//  Fixed-width layer schema. The per-event CSV always has kMaxLayers energy
//  columns; configs with fewer layers leave the extras at 0. This keeps the CSV
//  schema constant across the whole sweep so the aggregator never sees ragged
//  rows. (Dataset Schema.md: pad absent layers.)
//
//  Defined here (not in a header) and referenced via `extern` from EventAction.
// ----------------------------------------------------------------------------
extern const std::size_t kMaxLayers = 6;

// ----------------------------------------------------------------------------
RunAction::RunAction(const DetectorConstruction* det)
    : G4UserRunAction(), fDetector(det) {

    auto* mgr = G4AnalysisManager::Instance();
    mgr->SetDefaultFileType("csv");
    mgr->SetVerboseLevel(1);

    // Filename WITHOUT extension. By default "output" → "output.csv" in CWD.
    // The Python orchestrator overrides this per run with /analysis/setFileName
    // so each config writes into its own directory.
    mgr->SetFileName("output");

    // ---- define the ntuple (column order is the contract) ------------------
    mgr->CreateNtuple("Scoring", "Per-event Graded-Z scores");

    mgr->CreateNtupleIColumn("event_id");
    mgr->CreateNtupleDColumn("incident_energy_MeV");

    for (std::size_t i = 0; i < kMaxLayers; ++i) {
        std::ostringstream c; c << "layer" << i << "_edep_MeV";
        mgr->CreateNtupleDColumn(c.str());
    }

    mgr->CreateNtupleDColumn("detector_edep_MeV");

    mgr->CreateNtupleIColumn("n_primary_transmitted");
    mgr->CreateNtupleIColumn("n_secondary_photons");
    mgr->CreateNtupleIColumn("n_secondary_electrons");
    mgr->CreateNtupleIColumn("n_neutrons_transmitted");
    mgr->CreateNtupleIColumn("n_capture_gammas");

    mgr->CreateNtupleDColumn("transmitted_energy_MeV");
    mgr->CreateNtupleIColumn("n_transmitted_total");

    mgr->FinishNtuple();
}

// ----------------------------------------------------------------------------
void RunAction::BeginOfRunAction(const G4Run*) {
    G4AnalysisManager::Instance()->OpenFile();
}

// ----------------------------------------------------------------------------
void RunAction::EndOfRunAction(const G4Run* run) {
    auto* mgr = G4AnalysisManager::Instance();
    mgr->Write();
    mgr->CloseFile();
    // NOTE: deliberately NOT deleting the analysis manager. Framework owns it.

    if (IsMaster()) {
        G4cout << "\n--- Run finished: "
               << run->GetNumberOfEvent() << " events, "
               << fDetector->GetNumberOfLayers() << " layers. CSV written. ---\n"
               << G4endl;
    }
}
