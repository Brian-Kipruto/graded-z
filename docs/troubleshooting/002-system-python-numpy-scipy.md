# 002 — System Python: NumPy/SciPy binary mismatch, no scikit-learn

> Date: 2026-11-16 · Pass 00 · Cost: ~20 min

## Symptom
```
UserWarning: A NumPy version >=1.17.3 and <1.25.0 is required for this version of SciPy (detected version 2.2.6
ValueError: numpy.dtype size changed, may indicate binary incompatibility. Expected 96 from C header, got 88 from PyObject
ModuleNotFoundError: No module named 'sklearn'
```

## Cause
NumPy 2.2.6 installed in user site-packages (`pip --user`) shadowed the Debian NumPy that the Debian SciPy
in `/usr/lib/python3/dist-packages` was built against. scikit-learn was never installed. `requirements.txt`
is unpinned (D-21).

## Fix
```bash
sudo apt install -y python3.10-venv
cd ~/Desktop/graded-z && rm -rf .venv
python3 -m venv .venv && source .venv/bin/activate && pip install -r requirements.txt
```
`.venv/` is git-ignored. A first attempt without `python3.10-venv` failed, and because the block was not
chained with `&&`, `pip install` ran against user site-packages; stopped with Ctrl+C.

## Prevention
`source .venv/bin/activate` before any project Python (S-31). Chain setup steps with `&&` (S-29).
Pinning and packaging: Pass 14.
