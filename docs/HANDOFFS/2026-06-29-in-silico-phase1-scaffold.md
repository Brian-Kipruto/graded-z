# Handoff — In-Silico Phase 1 scaffold delivered (2026-06-29)

## What this delivery contains
A full vertical slice of Phase 1 (Sessions 2–9 collapsed) for hand-rewrite:

- **Session 2** — `PhysicsList` (EM_opt4 + FTFP_BERT_HP + ElasticHP + RadDecay
  and supporting constructors).
- **Session 3** — `RunAction` (CSV schema), `EventAction`, `EventData`,
  `G4AnalysisManager` ntuple.
- **Session 4** — three-layer parametric geometry + NaI(Tl) detector slab.
- **Session 5** — `DetectorMessenger` (`/shield/...` vocabulary) + rebuild.
- **Session 6** — Am-Be GPS source (`macros/source_ambe.mac`, mock spectrum).
- **Session 7** — `SteppingAction` particle classification by creator process.
- **Session 8** — Python orchestrator: LHS sampler, macro gen, joblib runner,
  CSV→parquet aggregation (`analysis/run_phase1.py`).

## How to verify (suggested order)
1. Build clean: `rm -rf build && mkdir build && cd build && cmake .. && make -j$(nproc)`.
2. Banner check (Session 2): run `./graded-z run_demo.mac`, confirm the physics
   banner lists the custom constructors, not bare FTFP_BERT.
3. CSV check (Session 3): confirm `demo_output.csv` has 1000 rows + the schema
   columns from RunAction.
4. Geometry check (Session 4/5): `./graded-z` interactive, confirm PE/Pb/Al
   stack + red detector slab, no overlap warnings.
5. Sweep check (Session 8): `python run_phase1.py --n 10 --jobs 4`, confirm
   `data/master.parquet` has 10 rows.

## Things deliberately left as decisions for you
- **Mock Am-Be spectrum** — the histogram in `source_ambe.mac` is a coarse
  ISO-8529-shaped placeholder. Swap for a measured/evaluated spectrum later.
- **Production cuts per region** — still global 0.7 mm. Tighten per-layer when
  energy-deposit resolution inside layers matters.
- **Composite density** — void-free rule of mixtures (ADR 0002). Override when
  lab densities exist.

## Landmines respected
No analysis-manager delete; G4Box half-extents; overlap checking true; explicit
unit math; GLOB requires `cmake ..` after new files.

## Next real session
Once lab recon completes: replace the source spectrum + slab dimensions with
measured values, re-run the sweep, and start Phase III training from
`analysis/ml/`.
EOF
