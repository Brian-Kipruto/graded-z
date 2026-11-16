#!/usr/bin/env python3
"""
run_phase1.py — one command to run the whole in-silico Phase 1 sweep.

Usage:
    python run_phase1.py --n 200 --jobs 8

Steps:
    1. sample N configs via Latin Hypercube
    2. generate macros + launch Geant4 in parallel (one dir per config)
    3. aggregate per-event CSVs into data/master.parquet
    4. print a tiny summary

This is the Session 8 deliverable: "one Python command runs N configs, produces
master CSV" — here producing parquet, per the locked schema.
"""
import argparse
import json

from orchestrator.sample import sample_configs
from orchestrator.runner import run_sweep
from orchestrator.aggregate import aggregate
from orchestrator import config as C


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=100, help="number of configs to sample")
    ap.add_argument("--jobs", type=int, default=-1, help="parallel workers (-1 = all cores)")
    ap.add_argument("--seed", type=int, default=0, help="sampler seed")
    ap.add_argument("--skip-run", action="store_true", help="aggregate only")
    args = ap.parse_args()

    if not C.BINARY.exists():
        raise SystemExit(f"Binary not found at {C.BINARY}. Build it first "
                         f"(cd build && cmake .. && make -j$(nproc)).")

    if not args.skip_run:
        print(f"Sampling {args.n} configs (LHS, seed={args.seed})...")
        configs = sample_configs(args.n, seed=args.seed)
        print(f"Launching sweep on {args.jobs} workers...")
        results = run_sweep(configs, n_jobs=args.jobs)
        ok = sum(1 for r in results if r["status"] == "ok")
        print(f"Sweep done: {ok}/{len(results)} succeeded.")

    print("Aggregating -> master.parquet ...")
    df = aggregate()
    print(df.head())
    print(f"\nShape: {df.shape}")


if __name__ == "__main__":
    main()
