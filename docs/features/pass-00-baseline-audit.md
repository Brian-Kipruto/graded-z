# Pass 00 — Baseline Audit

Code commit: 4a85968 · Reference tag: v0-dummy (= 8f85ae8) · Date: 2026-11-16 · Geant4 11.2.1 · Stage A

## What this pass built
THis was an almost no code build. By default this pass froze the dummy scafold I initially built(the one with the Pe/AL/Pb composite) and measures it so that every later fix is proven by a before/after number instead of an argument. 



| **Path** | **What** |
| :--- | :--- |
| `macros/tests/t01_gamma_empty.mac` | 4.4 MeV γ pencil beam, 10,000 events, "empty" stack (one 1 µm `G4_Galactic` layer). |
| `macros/tests/t02_neutron_empty.mac` | Same as `t01`, but with 4.5 MeV neutrons as a control. |
| `macros/tests/t03_source_distance.mac` | One Am-Be event at 10 cm and another at 50 cm, with verbose tracking. |
| `macros/tests/t04_seven_layers.mac` | 7 layers (Pb, 5× vacuum, Pb) where the CSV holds 6. |
| `macros/tests/t05_bad_command.mac` | Deliberate typo before `/run/beamOn`. |
| `macros/tests/t06_cuts.mac` | Pb/PE/Al stack; runs `/run/beamOn 0` then `/run/dumpCouples`. |
| `analysis/tools/csvsum.py` | Sums Geant4 CSV ntuple columns. Names parsed from `#column` lines (D-15). |
|.`gitignore`|	`!data/reference/*.csv, !tests/**/*`.csv exceptions (closes D-17)|
|`LICENSE, README.md`|	MIT licence; status / canonical repo / licence section (D-23)|
|docs/|	frozen-record skeleton: decisions/ (was adr/), features/, validation/, troubleshooting/, INDEX.md; docs/HANDOFFS/ removed (L-02)|

