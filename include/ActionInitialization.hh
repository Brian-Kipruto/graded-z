// ============================================================================
//  ActionInitialization.hh
// ----------------------------------------------------------------------------
//  The factory that wires up all user actions. Build() is called per worker
//  thread (and the master in MT mode). We run SerialOnly, so Build() runs once.
//
//  It needs a pointer to DetectorConstruction because several actions
//  (EventAction, SteppingAction, RunAction) query geometry info (layer count,
//  back-face z).
// ============================================================================
#ifndef ACTIONINITIALIZATION_HH
#define ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization {
public:
    explicit ActionInitialization(const DetectorConstruction* det);
    ~ActionInitialization() override = default;

    void Build() const override;          // worker-thread actions
    void BuildForMaster() const override; // master-thread actions (RunAction only)

private:
    const DetectorConstruction* fDetector;   // not owned
};

#endif  // ACTIONINITIALIZATION_HH
