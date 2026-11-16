"""
sample.py — configuration sampling via Latin Hypercube.

Why LHS (Python Orchestrator.md): full grid over 5 layers x materials x
thicknesses explodes to 100k+ sims (the curse of dimensionality the proposal
names). LHS with a few thousand well-spread points gives ML accuracy close to
full grid at a fraction of the compute.

Each sampled config is a dict:

    {
      "config_id": 42,
      "n_layers": 3,
      "source_distance_mm": 300.0,
      "layers": [
          {"material": "G4_POLYETHYLENE", "thickness_mm": 28.4},
          {"material": "G4_Pb",           "thickness_mm":  9.1},
          {"material": "G4_Al",           "thickness_mm": 12.0},
      ],
    }

The continuous dimensions (thicknesses) get the LHS treatment. The discrete
dimensions (n_layers, which material per slot) are sampled by mapping a uniform
LHS coordinate onto the integer/category range. This keeps everything inside one
space-filling design.
"""
from __future__ import annotations
from typing import List, Dict
import numpy as np
from scipy.stats import qmc

from . import config as C


def sample_configs(n_samples: int, seed: int = 0) -> List[Dict]:
    """Return a list of config dicts produced by a Latin Hypercube design."""
    rng = np.random.default_rng(seed)

    # We build one LHS over a fixed-width vector of dimensions:
    #   dim 0            : n_layers selector            (1 dim)
    #   dims 1..MAX      : material selector per slot    (MAX_LAYERS dims)
    #   dims MAX+1..2MAX : thickness per slot            (MAX_LAYERS dims)
    # Unused slots (beyond n_layers) are simply ignored for that config.
    n_dims = 1 + 2 * C.MAX_LAYERS
    sampler = qmc.LatinHypercube(d=n_dims, seed=seed)
    unit = sampler.random(n=n_samples)        # shape (n_samples, n_dims), in [0,1)

    configs: List[Dict] = []
    n_mat = len(C.MATERIAL_NAMES)

    for i in range(n_samples):
        row = unit[i]

        # --- n_layers: map [0,1) onto [MIN_LAYERS, MAX_LAYERS] -------------
        n_layers = C.MIN_LAYERS + int(
            row[0] * (C.MAX_LAYERS - C.MIN_LAYERS + 1)
        )
        n_layers = min(n_layers, C.MAX_LAYERS)

        layers = []
        for slot in range(n_layers):
            # material: map a unit coord onto a category index.
            mat_idx = int(row[1 + slot] * n_mat)
            mat_idx = min(mat_idx, n_mat - 1)
            material = C.MATERIAL_NAMES[mat_idx]

            # thickness: map a unit coord onto [MIN, MAX] mm.
            t = C.THICKNESS_MIN_MM + row[1 + C.MAX_LAYERS + slot] * (
                C.THICKNESS_MAX_MM - C.THICKNESS_MIN_MM
            )
            layers.append({"material": material, "thickness_mm": round(float(t), 3)})

        configs.append({
            "config_id": i,
            "n_layers": n_layers,
            "source_distance_mm": C.SOURCE_DISTANCE_MM,
            "layers": layers,
        })

    return configs


if __name__ == "__main__":
    # Quick smoke test: print the first few configs.
    import json
    cfgs = sample_configs(5, seed=1)
    print(json.dumps(cfgs, indent=2))
