// ============================================================================
//  EventAction.hh
// ----------------------------------------------------------------------------
//  Owns the per-event EventData accumulator. At end of event it:
//    1. reads the MFD energy-deposit hits maps (layer + detector),
//    2. merges them into EventData,
//    3. writes ONE CSV row via the analysis manager,
//    4. resets EventData for the next event.
//
//  EventData is shared with SteppingAction (which fills the classification
//  counters during the event). EventAction exposes it via Data().
// ============================================================================
#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "EventData.hh"
#include "globals.hh"

#include <vector>

class DetectorConstruction;
class G4Event;

class EventAction : public G4UserEventAction {
public:
    explicit EventAction(const DetectorConstruction* det);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    // SteppingAction reaches in here to fill the classification counters.
    EventData& Data() { return fData; }

private:
    const DetectorConstruction* fDetector;   // not owned, for layer count
    EventData fData;

    // Cached hits-collection IDs (resolved once, then reused). -1 = unresolved.
    std::vector<G4int> fLayerEdepHCID;        // one per layer
    G4int              fDetectorEdepHCID = -1;
};

#endif  // EVENTACTION_HH
