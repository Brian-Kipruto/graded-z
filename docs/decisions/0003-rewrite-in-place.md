# ADR 0003 — Rewrite in place

> Repo path: `docs/decisions/0003-rewrite-in-place.md` · Date: 2026-11-16 · Status: Accepted · Locks L-14 · Closes Q-01

## Context
A complete dummy scaffold exists (`8f85ae8`) and a code read found 24 defects. The thesis needs every file
understood by its author, who learns by retyping code by hand. The question was where the corrected code lives.

## Options considered
| Option | For | Against |
|---|---|---|
| **Rewrite in place on `main`**, dummy tagged `v0-dummy` | repo always builds and runs; `git diff v0-dummy -- <file>` shows exactly what was retyped and fixed; anchors compare old vs new in one tree | `main` mixes retyped and dummy files until Pass 22 |
| Fresh `rebuild/` tree beside the dummy | clean separation | two builds, two CMake projects; nothing runs until most of it exists; diffs across trees are awkward |
| New repository from scratch | cleanest history | throws away working orchestration; no running baseline to measure against |
| Patch the dummy without retyping | fastest | defeats the learning goal; the author never owns the code |

## Decision
Rewrite in place. The dummy is frozen as annotated tag `v0-dummy` (on `8f85ae8`). Each pass retypes the files
it touches and fixes that region's defects. By Pass 22 every dummy file has been retyped.

## Rejected options and why
The `rebuild/` tree was the main alternative. Rejected because a pass must end in a number, and a half-built
parallel tree cannot run end to end for many passes. Pass 00 itself depends on the running dummy as its
measured baseline. A new repository was rejected for the same reason, and because two repositories already
caused confusion (D-23).

## Consequences
+ Every pass has a running system and a before/after diff.
+ Pass 00 numbers are directly comparable with later anchors.
− Until Pass 22, a reader must check `git log -- <file>` to know whether a file is retyped or still dummy.

## Verification
Pass 00 measured the dummy in place (validation `pass-00-defect-baseline.md`). Pass 01 is the first retype;
its diff against `v0-dummy` is reviewed at close-out.
