"""
xgboost_model.py — Phase III primary model (stub).

Trains an XGBoost regressor on the dose_reduction_factor target as a starting
point. Real Phase III adds K-fold CV, Optuna tuning, and the DNN baseline.
"""
from __future__ import annotations
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error, r2_score

from orchestrator import config as C
from .features import build_feature_matrix


def train(target: str = "dose_reduction_factor"):
    df = pd.read_parquet(C.MASTER_PARQUET)
    X, y_dict = build_feature_matrix(df)
    y = y_dict[target]

    X_tr, X_te, y_tr, y_te = train_test_split(X, y, test_size=0.2, random_state=42)

    import xgboost as xgb
    model = xgb.XGBRegressor(n_estimators=400, max_depth=5, learning_rate=0.05)
    model.fit(X_tr, y_tr)

    pred = model.predict(X_te)
    print(f"target={target}  R2={r2_score(y_te, pred):.4f}  "
          f"MAE={mean_absolute_error(y_te, pred):.4f}")
    return model


if __name__ == "__main__":
    train()
