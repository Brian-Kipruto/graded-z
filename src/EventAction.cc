// ============================================================================
//  EventAction.cc
// ============================================================================
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4THitsMap.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

#include <sstream>

// ----------------------------------------------------------------------------
EventAction::EventAction(const DetectorConstruction* det)
    : G4UserEventAction(), fDetector(det) {}

// ----------------------------------------------------------------------------
void EventAction::BeginOfEventAction(const G4Event*) {
    // Reset the accumulator to the current layer count.
    fData.Reset(fDetector->GetNumberOfLayers());

    // Lazily size the cached-ID vector. We resolve the actual IDs on the first
    // EndOfEvent (collections don't exist until after the first event's hits
    // collection is created).
    if (fLayerEdepHCID.empty()) {
        fLayerEdepHCID.assign(fDetector->GetNumberOfLayers(), -1);
    }
}

// ----------------------------------------------------------------------------
//  Helper: sum a G4double hits map. Energy-deposit primitive scorers store one
//  G4THitsMap<G4double> keyed by copy number; we just want the total.
// ----------------------------------------------------------------------------
static G4double SumHitsMap(G4HCofThisEvent* hce, G4int hcID) {
    if (hcID < 0 || !hce) return 0.0;
    auto* hitsMap = static_cast<G4THitsMap<G4double>*>(hce->GetHC(hcID));
    if (!hitsMap) return 0.0;
    G4double total = 0.0;
    for (const auto& entry : *(hitsMap->GetMap())) {
        total += *(entry.second);
    }
    return total;
}

// ----------------------------------------------------------------------------
void EventAction::EndOfEventAction(const G4Event* event) {
    auto* sdMgr = G4SDManager::GetSDMpointer();
    auto* hce   = event->GetHCofThisEvent();

    // ---- resolve collection IDs once --------------------------------------
    for (std::size_t i = 0; i < fLayerEdepHCID.size(); ++i) {
        if (fLayerEdepHCID[i] < 0) {
            std::ostringstream col; col << "Layer" << i << "_MFD/Edep";
            fLayerEdepHCID[i] = sdMgr->GetCollectionID(col.str());
        }
    }
    if (fDetectorEdepHCID < 0) {
        fDetectorEdepHCID = sdMgr->GetCollectionID("Detector_MFD/Edep");
    }

    // ---- read energy deposits from the MFD hits maps ----------------------
    for (std::size_t i = 0; i < fData.layerEdep.size(); ++i) {
        fData.layerEdep[i] = SumHitsMap(hce, fLayerEdepHCID[i]);
    }
    fData.detectorEdep = SumHitsMap(hce, fDetectorEdepHCID);

    // ---- write ONE row to the ntuple --------------------------------------
    // Column order MUST match RunAction's CreateNtuple*Column order exactly.
    auto* mgr = G4AnalysisManager::Instance();
    G4int col = 0;

    // event id
    mgr->FillNtupleIColumn(col++, event->GetEventID());

    // incident energy (MeV) — keep units explicit (EventAction unit pitfall).
    mgr->FillNtupleDColumn(col++, fData.incidentEnergy / MeV);

    // per-layer energy deposit (MeV), fixed width = MAX_LAYERS (see RunAction).
    // We pad with 0.0 for absent layers so the schema is fixed-width.
    extern const std::size_t kMaxLayers;   // defined in RunAction.cc
    for (std::size_t i = 0; i < kMaxLayers; ++i) {
        G4double e = (i < fData.layerEdep.size()) ? fData.layerEdep[i] : 0.0;
        mgr->FillNtupleDColumn(col++, e / MeV);
    }

    // detector slab deposit (MeV)
    mgr->FillNtupleDColumn(col++, fData.detectorEdep / MeV);

    // classification counters (dimensionless integers)
    mgr->FillNtupleIColumn(col++, fData.nPrimaryTransmitted);
    mgr->FillNtupleIColumn(col++, fData.nSecondaryPhotons);
    mgr->FillNtupleIColumn(col++, fData.nSecondaryElectrons);
    mgr->FillNtupleIColumn(col++, fData.nNeutronsTransmitted);
    mgr->FillNtupleIColumn(col++, fData.nCaptureGammas);

    // transmitted energy (MeV) and count
    mgr->FillNtupleDColumn(col++, fData.transmittedEnergy / MeV);
    mgr->FillNtupleIColumn(col++, fData.nTransmittedTotal);

    mgr->AddNtupleRow();
}
