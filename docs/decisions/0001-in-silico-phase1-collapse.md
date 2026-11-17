# ADR 0001 — Collapse Sessions 2–9 into one in-silico Phase 1 codebase

## Status
Accepted (2026-06-29)

## Context
Lab access (INST recon) is pending. To avoid being blocked, we want a complete,
runnable mock of Phase 1 so that when real data arrives, only a few numbers
change (source spectrum, slab dimensions). The original plan staged this across
Sessions 2–9; this ADR records doing it in one pass for hand-rewrite practice.

## Decision
Build the full vertical slice now: custom PhysicsList (HP neutrons), parametric
geometry via Messenger, robust scoring (MFD energy deposit + SteppingAction
particle classification), Am-Be GPS source, and a Python LHS orchestrator that
sweeps configs and aggregates to parquet.

## Consequences
+ End-to-end contract is exercised immediately; integration bugs surface early.
+ Swapping in real lab data later is localized to source_ambe.mac and geometry.
- Larger up-front surface area to understand. Mitigated by heavy comments and
  rewriting by hand.

## Carry-forward landmines respected
- No `delete G4AnalysisManager::Instance()`.
- `G4Box` half-extents (halve once).
- Overlap checking `true` in all placements.
- Unit math explicit (`/MeV`, `* cm2` not `/(1.0/cm2)`).
- `cmake ..` re-run after adding files (GLOB).
