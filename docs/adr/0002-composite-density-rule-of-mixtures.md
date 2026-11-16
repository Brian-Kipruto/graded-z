# ADR 0002 — Composite density via void-free rule of mixtures

## Status
Accepted (2026-06-29)

## Context
Composite layers (polymer matrix + metal filler at a mass fraction) need an
effective density. The naive mass-weighted average of densities is WRONG.

## Decision
Use the void-free specific-volume rule:
    1/rho_mix = w_matrix/rho_matrix + w_filler/rho_filler

## Consequences
+ Physically correct for ideal mixtures; defensible in the committee.
- Real composites have voids. Override per-layer once lab densities arrive.
