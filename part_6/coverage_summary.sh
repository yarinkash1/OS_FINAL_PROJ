#!/usr/bin/env bash
set -euo pipefail

gcov -b server.cpp client.cpp > /dev/null || true

print_summary() {
  local file=$1
  echo "-- $file --"
  local total_lines covered_lines branches_total branches_taken
  total_lines=$(grep -E '^[[:space:]]*(#####|[0-9]+):' "$file.gcov" | wc -l || true)
  covered_lines=$(grep -E '^[[:space:]]*[1-9][0-9]*:' "$file.gcov" | wc -l || true)
  local pct="0.00"
  if [[ ${total_lines:-0} -gt 0 ]]; then
    pct=$(awk -v c="$covered_lines" -v t="$total_lines" 'BEGIN{printf("%.2f",100*c/t)}')
  fi
  branches_total=$(grep -E '^branch ' "$file.gcov" | wc -l || true)
  branches_taken=$(grep -E '^branch ' "$file.gcov" | grep -v 'taken 0' | wc -l || true)
  local bpct="0.00"
  if [[ ${branches_total:-0} -gt 0 ]]; then
    bpct=$(awk -v a="$branches_taken" -v b="$branches_total" 'BEGIN{printf("%.2f",100*a/b)}')
  fi
  echo "Lines: $covered_lines/$total_lines ($pct%)"
  echo "Branches: $branches_taken/$branches_total ($bpct%)"
  echo "Top uncovered (first 15):"
  grep -n '#####:' "$file.gcov" | head -n 15 || true
  echo
}

print_summary server.cpp
print_summary client.cpp

echo "(Use 'grep -n \"#####:\" server.cpp.gcov' for more)"