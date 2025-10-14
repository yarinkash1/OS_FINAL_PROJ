#!/usr/bin/env bash
set -euo pipefail

# Ensure gcov files exist for server and client
if [ ! -f server.cpp.gcov ] || [ ! -f client.cpp.gcov ]; then
  echo "gcov files missing; run gcov generation first" >&2
  exit 1
fi

get_pair() { # file
  local f=$1
  local lines_hit lines_tot branches_hit branches_tot
  # Lines: count executed vs total considered
  lines_tot=$(grep -E '^[[:space:]]*(#####|[0-9]+):' "$f" | wc -l || echo 0)
  lines_hit=$(grep -E '^[[:space:]]*[1-9][0-9]*:' "$f" | wc -l || echo 0)
  branches_tot=$(grep -E '^branch ' "$f" | wc -l || echo 0)
  branches_hit=$(grep -E '^branch ' "$f" | grep -v 'taken 0' | wc -l || echo 0)
  echo "$lines_hit $lines_tot $branches_hit $branches_tot"
}

read -r SL_HIT SL_TOT SB_HIT SB_TOT < <(get_pair server.cpp.gcov)
read -r CL_HIT CL_TOT CB_HIT CB_TOT < <(get_pair client.cpp.gcov)

TL_HIT=$((SL_HIT + CL_HIT))
TL_TOT=$((SL_TOT + CL_TOT))
TB_HIT=$((SB_HIT + CB_HIT))
TB_TOT=$((SB_TOT + CB_TOT))

line_pct=$(awk -v h=$TL_HIT -v t=$TL_TOT 'BEGIN{if(t>0) printf("%.2f", (h*100.0)/t); else print "0.00"}')
branch_pct=$(awk -v h=$TB_HIT -v t=$TB_TOT 'BEGIN{if(t>0) printf("%.2f", (h*100.0)/t); else print "0.00"}')

printf "Total Line Coverage: %d/%d = %s%%\n" "$TL_HIT" "$TL_TOT" "$line_pct"
printf "Total Branch Coverage: %d/%d = %s%%\n" "$TB_HIT" "$TB_TOT" "$branch_pct"
