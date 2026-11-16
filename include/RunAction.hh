// ============================================================================
//  RunAction.hh
// ----------------------------------------------------------------------------
//  Owns the CSV schema. BeginOfRun opens the file; EndOfRun writes + closes.
//  The ntuple column definitions here ARE the per-run output contract that the
//  Python aggregator depends on (Architecture Lockdown interface #2).
//
//  DO NOT delete G4AnalysisManager::Instance() anywhere — it is a framework
//  singleton; deleting double-frees on shutdown (the old-code crash).
// ============================================================================
#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"

class DetectorConstruction;
class G4Run;

class RunAction : public G4UserRunAction {
public:
    explicit RunAction(const DetectorConstruction* det);
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run* run) override;
    void EndOfRunAction(const G4Run* run) override;

private:
    const DetectorConstruction* fDetector;   // not owned
};

#endif  // RUNACTION_HH
