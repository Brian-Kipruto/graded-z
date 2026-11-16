"""
config.py — global parameters for the Graded-Z orchestrator.

Single source of truth for paths, the material palette, sweep bounds, and the
schema version. Everything else imports from here so there are no magic numbers
scattered across modules.
"""
from __future__ import annotations
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths. Adjust BINARY to wherever your build puts the executable.
# ---------------------------------------------------------------------------
PROJECT_ROOT = Path(__file__).resolve().parents[2]   # .../graded-z
BINARY       = PROJECT_ROOT / "build" / "graded-z"
DATA_DIR     = PROJECT_ROOT / "data"
RUNS_DIR     = DATA_DIR / "runs"
MASTER_PARQUET = DATA_DIR / "master.parquet"
FAILED_JSON    = DATA_DIR / "failed_runs.json"

# ---------------------------------------------------------------------------
# The material palette the sweep draws from. Each entry carries the metadata the
# ML feature table wants (effective Z, density) so the aggregator can attach it
# without re-deriving from Geant4. Z here is an "effective" atomic number used
# purely as an ML feature, not a physics input.
# ---------------------------------------------------------------------------
MATERIALS = {
    "G4_POLYETHYLENE": {"Z": 5,  "density_g_cm3": 0.94},   # low-Z moderator
    "G4_Pb":           {"Z": 82, "density_g_cm3": 11.34},  # high-Z absorber
    "G4_Al":           {"Z": 13, "density_g_cm3": 2.70},   # structural low-Z
    # Add more here (e.g. G4_W, G4_Bi) and the sweep picks them up automatically.
}
MATERIAL_NAMES = list(MATERIALS.keys())

# ---------------------------------------------------------------------------
# Sweep design space.
# ---------------------------------------------------------------------------
MIN_LAYERS        = 2
MAX_LAYERS        = 6        # MUST match kMaxLayers in RunAction.cc
THICKNESS_MIN_MM  = 1.0
THICKNESS_MAX_MM  = 50.0     # proposal §3.2.2.3: 1-50 mm per layer
SOURCE_DISTANCE_MM = 300.0   # 30 cm

EVENTS_PER_RUN    = 2000     # statistics per configuration

# ---------------------------------------------------------------------------
# Reproducibility / provenance.
# ---------------------------------------------------------------------------
SCHEMA_VERSION  = "v0"
PHYSICS_LIST_ID = "EM4_FTFP_BERT_HP_v1"
