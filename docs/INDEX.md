# docs/ — frozen record

Backward-looking: what was built and why, dated, not rewritten.
Forward-looking material (plans, handoffs, live decision list) lives in the vault, not here.

| Folder | Holds | Naming |
|---|---|---|
| `features/` | what each pass built; its retrospective | `pass-NN-<name>.md`, `pass-NN-<name>-retrospective.md` |
| `decisions/` | ADRs, one per architectural decision, rejected options included | `NNNN-<slug>.md` |
| `validation/` | one per measured checkpoint: numbers, method, code commit | `<test>.md` |
| `troubleshooting/` | only errors that cost real time | `NNN-<slug>.md` |

## Decisions
- [0001](decisions/0001-in-silico-phase1-collapse.md) — Collapse Sessions 2–9 into one in-silico Phase 1 codebase
- [0002](decisions/0002-composite-density-rule-of-mixtures.md) — Composite density: void-free rule of mixtures
- [0003](decisions/0003-rewrite-in-place.md) — Rewrite in place; dummy frozen as `v0-dummy` (Pass 00)

## Features
- [Pass 00 — Baseline Audit](features/pass-00-baseline-audit.md) · [retrospective](features/pass-00-baseline-audit-retrospective.md)

## Validation
- [Pass 00 defect baseline (M1–M11)](validation/pass-00-defect-baseline.md) — code `4a85968`

## Troubleshooting
- [001](troubleshooting/001-macro-typo-exit-zero.md) — Hand-typed macro fails silently with exit 0
- [002](troubleshooting/002-system-python-numpy-scipy.md) — System Python: NumPy/SciPy mismatch, no scikit-learn
