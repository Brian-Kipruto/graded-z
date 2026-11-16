"""
aggregate.py — collapse per-run, per-event CSVs into the master parquet.

This is where Layer 2 (per-run CSV) becomes Layer 3 (master dataset). Per the
Architecture Lockdown, ALL aggregation lives here in Python, NOT in
EndOfRunAction — so summaries can be recomputed without rebuilding C++.

For each config:
  - read output.csv (one row per event),
  - read meta.json (config + provenance),
  - compute the target variables (dose reduction, transmission, yields),
  - emit ONE row matching Dataset Schema.md.

Targets computed here (Scoring Strategy.md "per-configuration aggregates"):
  dose_reduction_factor     = mean(detector_edep) / mean(incident_energy)
  primary_transmission      = mean(n_primary_transmitted) / primaries_per_event
  secondary_photon_yield    = mean(n_secondary_photons) / primaries_per_event
  neutron_capture_gamma_yld = mean(n_capture_gammas)     / primaries_per_event
  mean_transmitted_energy   = sum(transmitted_energy) / sum(n_transmitted_total)
"""
from __future__ import annotations
import json
from pathlib import Path
from typing import Dict, List

import numpy as np
import pandas as pd

from . import config as C


def _primaries_per_event() -> float:
    """Average number of primaries GPS fires per event.

    With our Am-Be GPS (neutron intensity 1.0 + gamma intensity 0.6), GPS fires
    exactly ONE primary per event, chosen probabilistically between the two
    sources. So primaries_per_event = 1.0. Kept as a function so that if the
    source model changes (e.g. correlated emission), this is the one place to fix.
    """
    return 1.0


def _summarise_run(run_dir: Path) -> Dict | None:
    csv_path  = run_dir / "output.csv"
    meta_path = run_dir / "meta.json"
    if not csv_path.exists() or not meta_path.exists():
        return None

    meta = json.loads(meta_path.read_text())
    if meta.get("status") != "ok":
        return None

    # Geant4's CSV ntuple files start with comment/header lines prefixed by '#'.
    # The actual column header is the line listing the column names. We let
    # pandas skip comment lines and use the embedded header.
    try:
        df = pd.read_csv(csv_path, comment="#")
    except Exception:
        # Fallback: some Geant4 versions emit headers differently; try no-header.
        return None

    if df.empty:
        return None

    ppe = _primaries_per_event()

    mean_incident = df["incident_energy_MeV"].mean()
    mean_detector = df["detector_edep_MeV"].mean()

    dose_reduction = (mean_detector / mean_incident) if mean_incident > 0 else np.nan
    primary_transmission   = df["n_primary_transmitted"].mean() / ppe
    secondary_photon_yield = df["n_secondary_photons"].mean() / ppe
    capture_gamma_yield    = df["n_capture_gammas"].mean() / ppe

    total_trans_E = df["transmitted_energy_MeV"].sum()
    total_trans_N = df["n_transmitted_total"].sum()
    mean_trans_E  = (total_trans_E / total_trans_N) if total_trans_N > 0 else 0.0

    # --- assemble the fixed-width feature row -----------------------------
    row: Dict = {
        "config_id":          meta["config_id"],
        "n_layers":           meta["n_layers"],
        "source_distance_mm": meta["source_distance_mm"],
    }

    total_thickness = 0.0
    total_mass_areal = 0.0   # g/cm^2 = thickness_cm * density
    for i in range(C.MAX_LAYERS):
        if i < meta["n_layers"]:
            layer = meta["layers"][i]
            mat = layer["material"]
            t_mm = layer["thickness_mm"]
            info = C.MATERIALS[mat]
            row[f"layer_{i}_material"]      = mat
            row[f"layer_{i}_thickness_mm"]  = t_mm
            row[f"layer_{i}_Z"]             = info["Z"]
            row[f"layer_{i}_density"]       = info["density_g_cm3"]
            total_thickness += t_mm
            total_mass_areal += (t_mm / 10.0) * info["density_g_cm3"]
        else:
            # pad absent layers
            row[f"layer_{i}_material"]      = "none"
            row[f"layer_{i}_thickness_mm"]  = 0.0
            row[f"layer_{i}_Z"]             = 0
            row[f"layer_{i}_density"]       = 0.0

    row["total_thickness_mm"]   = round(total_thickness, 3)
    row["total_mass_g_per_cm2"] = round(total_mass_areal, 4)

    # --- targets ----------------------------------------------------------
    row["dose_reduction_factor"]       = dose_reduction
    row["primary_transmission"]        = primary_transmission
    row["secondary_photon_yield"]      = secondary_photon_yield
    row["neutron_capture_gamma_yield"] = capture_gamma_yield
    row["mean_transmitted_energy_MeV"] = mean_trans_E

    # --- metadata ---------------------------------------------------------
    row["n_events_simulated"] = meta["n_events"]
    row["geant4_version"]     = meta["geant4_version"]
    row["physics_list_id"]    = meta["physics_list_id"]
    row["wall_time_s"]        = meta.get("wall_time_s", np.nan)
    row["random_seed"]        = meta["random_seed"]
    row["schema_version"]     = meta["schema_version"]

    return row


def aggregate(runs_dir: Path = C.RUNS_DIR,
              out_path: Path = C.MASTER_PARQUET) -> pd.DataFrame:
    """Walk every config dir, summarise, write master parquet, return the df."""
    rows: List[Dict] = []
    for run_dir in sorted(runs_dir.glob("config_*")):
        summary = _summarise_run(run_dir)
        if summary is not None:
            rows.append(summary)

    df = pd.DataFrame(rows).sort_values("config_id").reset_index(drop=True)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    df.to_parquet(out_path, index=False)
    print(f"Aggregated {len(df)} configs -> {out_path}")
    return df


if __name__ == "__main__":
    aggregate()
