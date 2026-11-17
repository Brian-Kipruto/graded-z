# Validation — Pass 00 defect baseline (M1–M11)

> Repo path: `docs/validation/pass-00-defect-baseline.md` · **Code commit that produced the numbers:** `4a85968`
> (the binary measured is `v0-dummy` = `8f85ae8`; `4a85968` adds only macros and tooling) · Date: 2026-11-16

## What is tested and why it matters
The dummy scaffold, unchanged, measured against the suspected defects in the vault's Defect Register.
Every later fix is proven against these "before" values.

## Reference
No external reference values. Predictions come from reading the code at `8f85ae8`.
Geant4 behaviour cited from the v11.2.1 source: `G4RunMessenger.cc:189–193` (`/run/dumpCouples` valid
"after BeamOn has started"), `G4RunManager.cc:252–265, 299–308` (`beamOn 0` initialises without a user run),
`G4SDStructure.cc:80–86` (DET1010), `G4HCtable.cc:33–41` (duplicate collections reuse IDs),
`G4UIbatch.cc:222–227` (`Batch is interrupted`).

## Method
- Build: Geant4 11.2.1 (`geant4-11-02-patch-01 [MT]` libraries, serial run manager), `make` from clean
  objects at 2026-11-16 17:57 (the June binary was discarded, S-02).
- Physics list: dummy `PhysicsList` (0.7 mm global cut). World: air.
- "Empty" stack = one 1 µm `G4_Galactic` layer (Pass 00 decision 4).
- Macros: `macros/tests/t01`–`t06`, plus `build/run_demo.mac` (unchanged).
- Seeds: t01 (1001, 2001), t02 (1002, 2002), t03 (1003, 2003), t04 (1004, 2004), demo (12345, 67890).
- Sums: `analysis/tools/csvsum.py`.
- Python env for M7 (repo `.venv/`, Python 3.10): joblib 1.6.0, numpy 2.2.6, pandas 2.3.3, pyarrow 25.0.1,
  scikit-learn 1.7.2, scipy 1.15.3, xgboost 3.2.0.

## Results
| Test | Defect | Prediction | Measured | Verdict |
|---|---|---|---|---|
| M1 | D-06 (C++) | `demo_output_nt_Scoring.csv` | `demo_output_nt_Scoring.csv` | **confirmed** |
| M2 | D-01 | γ, 10 000 ev: Σ`n_primary_transmitted` = 0; Σ`n_secondary_photons` ≈ 10 000 | **0**; **9 996** (+ 6 e⁻) | **confirmed** |
| M2 control | — | n, 10 000 ev: Σ`n_primary_transmitted` ≈ 10 000 | **9 992** (Σ`n_neutrons_transmitted` 9 993) | neutron branch counts primaries |
| M3 | D-04 | Σ`n_transmitted_total`/10 000 > 1 | γ **1.0002**; n **0.9995** | **not observed** ≥ 10⁻⁴; γ excess = 6 secondary e⁻; one non-primary neutron crossing unexplained. Test cannot separate re-crossing from secondaries. |
| M4 | D-02 | first-step z = −300 mm at 10 cm and 50 cm | **−300, −300** mm | **confirmed** |
| M5 | D-05 | exit 0; "7 layers"; 6 layer columns; Σ`layer0_edep` > 0 | exit 0; **7 layers**; **6** columns; **1 327.42 MeV** | **confirmed**; layer-6 Edep has no column |
| M6 | D-07 | DET1010 count > 0 | **2 warnings** (4 matching lines). Text: `Layer0_MFD had already been stored in /. Object pointer is overwritten. It's users' responsibility to delete the old sensitive detector object.` | **confirmed as warning**; no data effect (M5 Edep read after rebuild) |
| M7 | D-06 (Py) | 3/3 "ok", aggregator fails | `3/3 succeeded` (all three logs show `Run finished: 2000 events`), then `KeyError: 'config_id'` at `aggregate.py:137` | **confirmed** |
| M8 | — | baseline | `run_demo.mac`, 1000 events: **2.80 s** real, 2.69 s user | baseline for P19 |
| M9 | D-15 | only `#` header lines, 1000 rows | **20** `#` lines (16 `#column`), **no plain header row**, **1000** rows | **confirmed** |
| M10 | D-16 | exit 0, no CSV | exit **0**; `COMMAND NOT FOUND </gps/directoin 0 0 1>`; `Batch is interrupted`; no CSV | **confirmed** (also hit by accident, t02) |
| M11 | D-22 | γ threshold in Pb ≈ 0.1 MeV | see table below | **baseline recorded** |
| — | D-17 | `data/reference/*.csv` not ignored after fix | before: ignored by `*.csv`; after: matched by `!data/reference/*.csv` (not ignored) | **closed** |

**M11: production thresholds at a 0.7 mm range cut (all particles)**

| Material | γ | e⁻ | e⁺ | proton |
|---|---|---|---|---|
| G4_AIR | 990 eV | 990 eV | 990 eV | 70 keV |
| G4_POLYETHYLENE | 1.976 keV | 277.6 keV | 271.0 keV | 70 keV |
| G4_Pb | **94.70 keV** | 1.007 MeV | 946.9 keV | 70 keV |
| G4_Al | 5.847 keV | 461.2 keV | 445.1 keV | 70 keV |
| G4_SODIUM_IODIDE | 23.98 keV | 476.6 keV | 457.0 keV | 70 keV |

## Verdict
Pass criterion (every row measured; each defect confirmed or refuted in writing): **YES**.

## Anchor frozen
None yet. The anchor suite starts at Pass 13; these values are its "dummy" column.
