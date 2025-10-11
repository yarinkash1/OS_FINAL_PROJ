#!/bin/bash
set -e

# 1) No args -> usage
./main || true

# 2) Missing one required arg -> usage (missing -s)
./main -v 4 -e 4 || true

# 3) Invalid option -> getopt returns '?'
./main -v 4 -e 4 -z 42 || true

# 4) vertices <= 0
./main -v 0 -e 1 -s 1 || true

# 5) edges < 0
./main -v 3 -e -1 -s 1 || true

# 6) edges > maxEdges -> clamp warning path
./main -v 5 -e 50 -s 7 || true

# 7) valid run (likely Eulerian)
./main -v 4 -e 4 -s 4 || true

# 8) another valid run (different seed, maybe non-Eulerian)
./main -v 4 -e 4 -s 42 || true

# 9) (optional) force a non-Eulerian structure to exercise failure path in algorithm
./main -v 4 -e 3 -s 1 || true
