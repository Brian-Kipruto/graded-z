"""
features.py — Phase III feature engineering (kept separate from Geant4 output).

Per Dataset Schema.md, feature engineering happens at TRAIN time, not Geant4
time. The master parquet holds raw values; this module turns them into an ML
matrix: one-hot encode the categorical layer-material columns, scale the
continuous columns. Stubbed minimally here so Phase III has a starting point.
"""
from __future__ import annotations
import pandas as pd
from sklearn.preprocessing import StandardScaler


def build_feature_matrix(df: pd.DataFrame):
    """Return (X, y_dict) ready for XGBoost/DNN training."""
    material_cols = [c for c in df.columns if c.endswith("_material")]
    X = pd.get_dummies(df, columns=material_cols, prefix=material_cols)

    target_cols = [
        "dose_reduction_factor", "primary_transmission",
        "secondary_photon_yield", "neutron_capture_gamma_yield",
        "mean_transmitted_energy_MeV",
    ]
    meta_cols = [
        "config_id", "geant4_version", "physics_list_id", "wall_time_s",
        "random_seed", "schema_version", "n_events_simulated",
    ]
    y = {t: df[t] for t in target_cols if t in df}
    X = X.drop(columns=[c for c in target_cols + meta_cols if c in X.columns])

    # Scale continuous numeric features (one-hot dummies pass through fine).
    num_cols = X.select_dtypes(include="number").columns
    X[num_cols] = StandardScaler().fit_transform(X[num_cols])
    return X, y
