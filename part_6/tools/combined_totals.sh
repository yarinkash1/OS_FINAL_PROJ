#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Usage: combined_totals.sh server.cpp.gcov client.cpp.gcov" >&2
  exit 1
fi

SERVER_GCOV="$1"
CLIENT_GCOV="$2"

# Derive totals using coverage_summary.sh output (run in parent directory)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
SUMMARY_OUTPUT="$(bash "$PARENT_DIR/coverage_summary.sh" 2>/dev/null)"

# Extract sections by simple pattern matching
SLINE="$(echo "$SUMMARY_OUTPUT" | awk '/-- server.cpp --/{f=1;next} /-- client.cpp --/{f=0} f && /Lines:/{print;exit}')" || true
CLINE="$(echo "$SUMMARY_OUTPUT" | awk '/-- client.cpp --/{f=1;next} /Use/{f=0} f && /Lines:/{print;exit}')" || true
SBR="$(echo "$SUMMARY_OUTPUT" | awk '/-- server.cpp --/{f=1;next} /-- client.cpp --/{f=0} f && /Branches:/{print;exit}')" || true
CBR="$(echo "$SUMMARY_OUTPUT" | awk '/-- client.cpp --/{f=1;next} /Use/{f=0} f && /Branches:/{print;exit}')" || true

parse_pair() { # expects e.g. 'Lines: 84/92 (91.30%)'
  echo "$1" | sed -E 's/.*: *([0-9]+)\/([0-9]+).*/\1 \2/'
}

read -r SL_HIT SL_TOT <<<"$(parse_pair "$SLINE" 2>/dev/null || echo "0 0")"
read -r CL_HIT CL_TOT <<<"$(parse_pair "$CLINE" 2>/dev/null || echo "0 0")"
read -r SB_HIT SB_TOT <<<"$(parse_pair "$SBR" 2>/dev/null || echo "0 0")"
read -r CB_HIT CB_TOT <<<"$(parse_pair "$CBR" 2>/dev/null || echo "0 0")"

TL_HIT=$((SL_HIT + CL_HIT))
TL_TOT=$((SL_TOT + CL_TOT))
TB_HIT=$((SB_HIT + CB_HIT))
TB_TOT=$((SB_TOT + CB_TOT))

if [ "$TL_TOT" -gt 0 ]; then
  printf "Total Line Coverage: %d/%d = %.2f%%\n" "$TL_HIT" "$TL_TOT" "$(awk -v h=$TL_HIT -v t=$TL_TOT 'BEGIN{printf("%.2f", (h*100.0)/t)}')"
else
  echo "Total Line Coverage: 0/0 = 0.00%"
fi

if [ "$TB_TOT" -gt 0 ]; then
  printf "Total Branch Coverage: %d/%d = %.2f%%\n" "$TB_HIT" "$TB_TOT" "$(awk -v h=$TB_HIT -v t=$TB_TOT 'BEGIN{printf("%.2f", (h*100.0)/t)}')"
else
  echo "Total Branch Coverage: 0/0 = 0.00%"
fi
