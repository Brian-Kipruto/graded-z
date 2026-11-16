"""
runner.py — launch Geant4 subprocesses in parallel, one per config.

Each config gets its own directory (never share a working dir — pitfall). We
write config.mac, a meta.json (provenance, written BEFORE launch), invoke the
binary in batch mode, capture stdout.log, and record wall-time. Failures are
logged and skipped, not fatal, so a 5000-config sweep survives a few bad
geometries.
"""
from __future__ import annotations
import json
import subprocess
import time
from pathlib import Path
from typing import Dict, List

from joblib import Parallel, delayed

from . import config as C
from .macros import write_macro


def _git_sha() -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=C.PROJECT_ROOT, text=True
        ).strip()
    except Exception:
        return "unknown"


def _geant4_version() -> str:
    try:
        return subprocess.check_output(
            ["geant4-config", "--version"], text=True
        ).strip()
    except Exception:
        return "unknown"


# Resolve provenance once (cheap, avoids per-run subprocess spawns).
_GIT_SHA = _git_sha()
_G4_VER  = _geant4_version()


def run_one(cfg: Dict, seed: int, timeout_s: int = 1800) -> Dict:
    """Run a single configuration. Returns a result dict (never raises)."""
    run_dir = C.RUNS_DIR / f"config_{cfg['config_id']:05d}"
    run_dir.mkdir(parents=True, exist_ok=True)

    mac_path = write_macro(cfg, run_dir, seed)

    # Provenance written BEFORE launch (so a crash still leaves a record).
    meta = {
        "config_id": cfg["config_id"],
        "n_layers": cfg["n_layers"],
        "layers": cfg["layers"],
        "source_distance_mm": cfg["source_distance_mm"],
        "n_events": C.EVENTS_PER_RUN,
        "random_seed": seed,
        "schema_version": C.SCHEMA_VERSION,
        "physics_list_id": C.PHYSICS_LIST_ID,
        "geant4_version": _G4_VER,
        "git_sha": _GIT_SHA,
    }
    (run_dir / "meta.json").write_text(json.dumps(meta, indent=2))

    log_path = run_dir / "stdout.log"
    t0 = time.time()
    status = "ok"
    try:
        with log_path.open("w") as log:
            subprocess.run(
                [str(C.BINARY), str(mac_path)],
                stdout=log, stderr=subprocess.STDOUT,
                cwd=str(C.PROJECT_ROOT / "build"),   # so it finds source_ambe.mac
                timeout=timeout_s, check=True,
            )
    except subprocess.TimeoutExpired:
        status = "timeout"
    except subprocess.CalledProcessError as e:
        status = f"error_rc_{e.returncode}"
    except Exception as e:  # noqa: BLE001
        status = f"exception_{type(e).__name__}"

    wall = time.time() - t0
    meta["wall_time_s"] = round(wall, 3)
    meta["status"] = status
    (run_dir / "meta.json").write_text(json.dumps(meta, indent=2))

    return {"config_id": cfg["config_id"], "status": status, "wall_time_s": wall}


def run_sweep(configs: List[Dict], n_jobs: int = -1, base_seed: int = 1000) -> List[Dict]:
    """Run all configs in parallel. base_seed offsets per-config seeds."""
    C.RUNS_DIR.mkdir(parents=True, exist_ok=True)

    results = Parallel(n_jobs=n_jobs, verbose=10)(
        delayed(run_one)(cfg, seed=base_seed + cfg["config_id"])
        for cfg in configs
    )

    # Record failures separately for quick triage.
    failed = [r for r in results if r["status"] != "ok"]
    if failed:
        C.FAILED_JSON.write_text(json.dumps(failed, indent=2))
    return results
