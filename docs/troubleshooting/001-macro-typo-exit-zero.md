# 001 — Hand-typed macro fails silently with exit 0

> Date: 2026-11-16 · Pass 00 · Cost: ~15 min

## Symptom
`./graded-z ../macros/tests/t02_neutron_empty.mac; echo $?` printed `exit 0`, but no
`t02_neutron_empty_nt_Scoring.csv` appeared. The tail of the log showed only vis-manager shutdown text.

## Cause
A hand-typing slip: `/gps/ direction 0 0 1`. Geant4 printed `***** COMMAND NOT FOUND … *****` and
`***** Batch is interrupted!! *****`, then aborted the macro before `/run/beamOn`. `main.cc` ignores the
`ApplyCommand` status and returns 0 (D-16, fixed in P08).

## Diagnosis
```bash
grep -nE "COMMAND NOT FOUND|interrupted" t02.log
```

## Fix
Re-paste the macro, or compare the commands only:
```bash
grep -v '^\s*#' FILE | grep -v '^\s*$' | md5sum
```

## Prevention
After every run: `grep -E "Run finished|interrupted" <log>`. Never accept an exit code alone (S-21).
