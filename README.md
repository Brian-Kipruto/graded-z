# Graded-Z Shielding — In-Silico Phase 1

A complete, runnable mock of Phase 1 for the MSc project *"Application of Machine
Learning for Rapid Performance Prediction of Flexible Graded-Z Composites."*
It collapses the planned Sessions 2–9 into one vertical slice so the whole
data-generation contract works end to end before lab data arrives.

When real Am-Be measurements come in, only two things change: the spectrum in
`macros/source_ambe.mac` and the geometry numbers fed by the sweep.

## Layout

```
graded-z/
├── CMakeLists.txt
├── main.cc                     # entry point: interactive or batch
├── include/ , src/             # the C++ Geant4 classes
├── macros/
│   ├── vis.mac                 # interactive visualization
│   ├── source_ambe.mac         # mock Am-Be neutron+gamma spectrum (GPS)
│   └── run_demo.mac            # one self-contained batch run
├── analysis/
│   ├── run_phase1.py           # ONE command: sample → sweep → aggregate
│   ├── orchestrator/           # sampling, macros, runner, aggregation
│   └── ml/                     # Phase III feature + model stubs
└── docs/                       # frozen record: decisions/ features/ validation/ troubleshooting/ (see docs/INDEX.md)
```

## The C++ classes (read order for rewriting by hand)

1. `LayerSpec.hh`            — the geometry data contract.
2. `DetectorConstruction`    — world, parametric stack, NaI detector slab.
3. `PhysicsList`             — custom modular list, HP neutrons (Session 2).
4. `PrimaryGeneratorAction`  — Am-Be via GPS (spectrum lives in the macro).
5. `RunAction`               — defines the CSV schema (the per-run contract).
6. `EventData` + `EventAction` + `SteppingAction` — the SCORING core.
7. `DetectorMessenger`       — the keystone: macro vocabulary → geometry.
8. `ActionInitialization` + `main.cc` — wiring.

## Build

```bash
mkdir build && cd build
cmake ..              # re-run this whenever you ADD a new .cc/.hh (GLOB gotcha)
make -j$(nproc)
```

## Run

Interactive (see the stack + tracks):
```bash
cd build && ./graded-z
```

One batch config by hand:
```bash
cd build && ./graded-z run_demo.mac      # writes demo_output.csv
```

The full sweep (the Session 8 deliverable):
```bash
cd analysis
pip install -r ../requirements.txt
python run_phase1.py --n 200 --jobs 8
# → data/runs/config_XXXXX/  (config.mac, output.csv, meta.json, stdout.log)
# → data/master.parquet      (one row per config, ML-ready)
```

## How data is gathered (the part to understand deeply)

Two complementary mechanisms, by design:

- **Energy deposit** — `G4MultiFunctionalDetector` + `G4PSEnergyDeposit` on each
  layer and on the detector slab. Leak-proof, framework-blessed. Read in
  `EventAction::EndOfEventAction` from the hits maps.
- **Particle classification** — `SteppingAction` watches every step for a track
  crossing the stack back face in +z, then bins it by particle type and creator
  process (`eBrem`, `nCapture`, …). This is what primitive scorers cannot do.

Both feed one `EventData` struct → one CSV row per event. Python then aggregates
per-event rows into one per-config row of targets (dose reduction, transmission,
secondary yields) in `aggregate.py` — never in C++.

## Known landmines (do not reintroduce)

- Never `delete G4AnalysisManager::Instance()`.
- `G4Box` takes HALF extents — halve once.
- Pass `true` for overlap checking in every `G4PVPlacement` during dev.
- Keep unit math explicit: `value / MeV`, `flux * cm2` (NOT `/(1.0/cm2)`).
- After adding a new source file, re-run `cmake ..` before `make`.

## Status, repository and licence

- **Status:** under rewrite-in-place. The original scaffold is frozen at tag `v0-dummy`;
  each pass retypes the files it touches and fixes their measured defects.
- **Canonical repository:** `github.com/Brian-Kipruto/graded-z`. The older
  `graded-z-shielding` repository is archived and superseded by this one.
- **Licence:** code under the MIT licence (see `LICENSE`). The dataset will be released
  under CC-BY-4.0.
