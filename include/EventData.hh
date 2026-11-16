// ============================================================================
//  EventData.hh
// ----------------------------------------------------------------------------
//  A plain accumulator for everything we score in ONE event. SteppingAction
//  fills it as the event unfolds; EventAction reads it at end-of-event, writes
//  one CSV row, then resets it for the next event.
//
//  Keeping this in its own struct (rather than as members scattered across
//  EventAction) makes the data contract explicit and easy to extend: add a
//  field here, fill it in SteppingAction, write it in EventAction, declare the
//  column in RunAction. Four edits, one new feature.
//
//  WHAT WE SCORE AND WHY (Scoring Strategy.md "the right way"):
//    Per-layer energy deposit comes from the MFD primitive scorers (read in
//    EventAction). Everything below is the particle-classification half that
//    primitive scorers can't do — it needs creator-process inspection in
//    SteppingAction.
//
//  "Back face" = the plane at z = stackBackZ. A particle counts as TRANSMITTED
//  the first step it crosses that plane moving in +z. We tag by:
//    - primary survivor (TrackID == 1 reaching the back face)
//    - secondary photon (gamma not from the source)
//    - secondary electron
//    - neutron
//    - capture gamma (gamma whose creator process is "nCapture")
// ============================================================================
#ifndef EVENTDATA_HH
#define EVENTDATA_HH

#include "globals.hh"
#include <vector>

struct EventData {
    // --- per-layer energy deposit (MeV). Index = layer number. --------------
    // Sized to the number of layers at BeginOfRun; EventAction also reads the
    // MFD hits maps, but we keep a vector here for the SteppingAction path and
    // for symmetry. (We use the MFD values as the authoritative Edep.)
    std::vector<G4double> layerEdep;     // filled by EventAction from MFD

    // --- detector slab energy deposit (MeV) ---------------------------------
    G4double detectorEdep = 0.0;

    // --- particles crossing the back face (counts) --------------------------
    G4int  nPrimaryTransmitted   = 0;    // primary that survived the whole stack
    G4int  nSecondaryPhotons     = 0;    // brems / fluorescence gammas out the back
    G4int  nSecondaryElectrons   = 0;
    G4int  nNeutronsTransmitted  = 0;
    G4int  nCaptureGammas        = 0;    // gammas created by nCapture, out the back

    // --- energy carried out the back face (MeV) -----------------------------
    G4double transmittedEnergy   = 0.0;  // summed KE of everything exiting +z
    G4int    nTransmittedTotal   = 0;    // count, for computing mean energy

    // --- incident energy bookkeeping (MeV) ----------------------------------
    // Summed KE of all primaries generated this event. Lets EventAction compute
    // dose_reduction = detectorEdep / incidentEnergy without guessing.
    G4double incidentEnergy      = 0.0;

    void Reset(std::size_t nLayers) {
        layerEdep.assign(nLayers, 0.0);
        detectorEdep        = 0.0;
        nPrimaryTransmitted = 0;
        nSecondaryPhotons   = 0;
        nSecondaryElectrons = 0;
        nNeutronsTransmitted= 0;
        nCaptureGammas      = 0;
        transmittedEnergy   = 0.0;
        nTransmittedTotal   = 0;
        incidentEnergy      = 0.0;
    }
};

#endif  // EVENTDATA_HH
