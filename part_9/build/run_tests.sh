#!/bin/bash
# Note:
#  - Set SKIP_HEAVY=1 to skip very large inputs (e.g., V=70000)
#  - Set QUICK=1 to run a minimal smoke suite and compute coverage quickly
set -euo pipefail

PORT="${PORT:-9090}"
SERVER_PID=""
SERVER_KILLED=0 

LOG_DIR="build"
if [[ "$(basename "$PWD")" == "build" ]]; then
  LOG_DIR="testlogs"
fi
mkdir -p "$LOG_DIR"

flush_coverage() {
  [[ -n "${SERVER_PID}" ]] || return 0
  kill -0 "${SERVER_PID}" 2>/dev/null || return 0
  [[ "${SERVER_KILLED}" -eq 1 ]] && return 0

  kill -USR1 "${SERVER_PID}" 2>/dev/null || true
  sleep 0.1

}

restart_server() {
  flush_coverage
  printf "SHUTDOWN\n" | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" >/dev/null 2>&1 || true
  sleep 0.3
  wait "$SERVER_PID" 2>/dev/null || true
  ./server "$PORT" > "$LOG_DIR/server.out" 2> "$LOG_DIR/server.err" &
  SERVER_PID=$!
  sleep 0.5
}




cleanup() {
  [[ -n "${SERVER_PID}" ]] || return 0
  if kill -0 "${SERVER_PID}" 2>/dev/null; then
    flush_coverage
    kill -TERM "${SERVER_PID}" 2>/dev/null || true
    sleep 0.5
    flush_coverage
    timeout 2s bash -c "while kill -0 ${SERVER_PID} 2>/dev/null; do sleep 0.1; done" || true
    if kill -0 "${SERVER_PID}" 2>/dev/null; then
      SERVER_KILLED=1
      kill -KILL "${SERVER_PID}" 2>/dev/null || true
    fi
  fi
  wait "${SERVER_PID}" 2>/dev/null || true
}




trap cleanup EXIT

echo "[1] Starting server..."

if command -v fuser >/dev/null 2>&1; then
  fuser -k "${PORT}/tcp" 2>/dev/null || true
fi

./server "$PORT" > "$LOG_DIR/server.out" 2> "$LOG_DIR/server.err" &
SERVER_PID=$!
export SERVER_PID
sleep 0.5

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
  echo "[!] Server failed to start; dumping $LOG_DIR/server.err"
  cat "$LOG_DIR/server.err" || true
  exit 1
fi
if command -v nc >/dev/null 2>&1; then
  if ! timeout 2s nc -z 127.0.0.1 "$PORT"; then
    echo "[!] Port $PORT is not listening; dumping $LOG_DIR/server.err"
    cat "$LOG_DIR/server.err" || true
    exit 1
  fi
fi

NC_CLOSE_OPT="-N"; nc -h 2>&1 | grep -q -- "-N" || NC_CLOSE_OPT="-q 1"
run_with_input() { local in="$1" out="$2" err="$3"; timeout 15s ./client < "$in" > "$out" 2> "$err" || true; }

echo "[2] Client quick exit"
cat > "$LOG_DIR/input_quick_exit.txt" <<'EOF'
0
EOF
run_with_input "$LOG_DIR/input_quick_exit.txt" "$LOG_DIR/client_quick_exit.out" "$LOG_DIR/client_quick_exit.err"

echo "[2.1] Client connect error path"
PORT=65530 timeout 3s ./client 65530 > "$LOG_DIR/client_connect_fail.out" 2> "$LOG_DIR/client_connect_fail.err" || true

# --- EARLY SMOKE to force server.gcda ---
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" > /dev/null 2>&1 || true

flush_coverage
printf "SHUTDOWN\n" | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" > /dev/null 2>&1 || true
sleep 0.2
wait "$SERVER_PID" 2>/dev/null || true

# Fast path: if QUICK=1, stop here and compute coverage totals
if [ "${QUICK:-0}" = "1" ]; then
  echo "[quick] QUICK=1 set; skipping full suite and computing coverage now."
  echo "=== GCOV (post-run) ==="
  # Show produced .gcda files
  find . -name '*.gcda' | sed 's/^/  /'

  # Compute aggregate totals without generating .gcov files
  if ls ./*.gcda >/dev/null 2>&1; then
    gcov -b -n ./*.gcda 2>/dev/null | awk '
      BEGIN { lh=0; lt=0; bh=0; bt=0 }
      /Lines executed:/ {
        if (match($0, /Lines executed: *([0-9.]+)% of *([0-9]+)/, a)) {
          lt += a[2];
          # round to nearest int to reduce precision drift
          lh += int(a[1]*a[2]/100 + 0.5);
        }
      }
      /Taken at least once:/ {
        if (match($0, /Taken at least once: *([0-9.]+)% of *([0-9]+)/, c)) {
          bt += c[2];
          bh += int(c[1]*c[2]/100 + 0.5);
        }
      }
      END {
        lp = (lt ? 100.0*lh/lt : 0);
        bp = (bt ? 100.0*bh/bt : 0);
        printf("Total Line Coverage: %d/%d = %.2f%%\n", lh, lt, lp);
        printf("Total Branch Coverage: %d/%d = %.2f%%\n", bh, bt, bp);
      }
    '
  fi

  echo " All test runs completed."
  exit 0
fi

echo "[1b] Restarting server for full suite..."
./server "$PORT" > "$LOG_DIR/server.out" 2> "$LOG_DIR/server.err" &
SERVER_PID=$!
sleep 0.5


# 1 (ALL menu) → undirected → preview only → back to menu → 0
echo "[3] Undirected PREVIEW only"
cat > "$LOG_DIR/input_undirected_preview_only.txt" <<'EOF'
1
0
6
7
1
5
-1
3
0
0
EOF
run_with_input "$LOG_DIR/input_undirected_preview_only.txt" \
  "$LOG_DIR/client_undirected_preview_only.out" "$LOG_DIR/client_undirected_preview_only.err"

# Directed PREVIEW -> run ALL (same seed) -> exit
echo "[4] Directed PREVIEW -> ALL"
cat > "$LOG_DIR/input_directed_preview_all.txt" <<'EOF'
1
1
3
5
2
2
-1
2
1

0
EOF
run_with_input "$LOG_DIR/input_directed_preview_all.txt" \
  "$LOG_DIR/client_directed_preview_all.out" "$LOG_DIR/client_directed_preview_all.err"
echo "[4.1] Directed PREVIEW -> ALL (valid for all)"
cat > "$LOG_DIR/input_directed_valid_all.txt" <<'EOF'
1
1
5
6
1
5
0
4
3
1
1
EOF
run_with_input "$LOG_DIR/input_directed_valid_all.txt" \
  "$LOG_DIR/client_directed_valid_all.out" "$LOG_DIR/client_directed_valid_all.err"

# Invalid numbers + out-of-range + non-digits (exercises prompt_int)
echo "[5] Client invalid numbers + range + alpha"
cat > "$LOG_DIR/input_invalid_numbers.txt" <<'EOF'
1
0
a
5
6
-2
2
-1
3
0
0
EOF
run_with_input "$LOG_DIR/input_invalid_numbers.txt" \
  "$LOG_DIR/client_invalid_numbers.out" "$LOG_DIR/client_invalid_numbers.err"

echo "[5.1] Client prompt_int tricky cases"
cat > "$LOG_DIR/input_prompt_tricky.txt" <<'EOF'
1
0
     
+5
5
3
-      
2
-1      
2
99999999999999999999999999
4
0       
1
0       
1
-1      # SRC=-1 (skip)
-1
-1      # K=-1 (skip)
0
EOF
run_with_input "$LOG_DIR/input_prompt_tricky.txt" \
  "$LOG_DIR/client_prompt_tricky.out" "$LOG_DIR/client_prompt_tricky.err"


# Overflow / extreme values / out-of-range paths in prompt_int
echo "[6] Client prompt_int overflow/invalid/out-of-range"
cat > "$LOG_DIR/input_prompt_overflow.txt" <<'EOF'
1
1
9999999999
4
1
1
-1
2
0
0
EOF
run_with_input "$LOG_DIR/input_prompt_overflow.txt" \
  "$LOG_DIR/client_prompt_overflow.out" "$LOG_DIR/client_prompt_overflow.err"

# ============================  Raw TCP tests (netcat)  ===============
# Use timeout + $NC_CLOSE_OPT + -w 2 everywhere to avoid hangs.

echo "[7] Unknown directive (raw)"
printf "HELLO\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_unknown_directive.out" 2> "$LOG_DIR/raw_unknown_directive.err" || true

echo "[8] Missing V (raw)"
printf "ALG MAX_FLOW\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_missing_v.out" 2> "$LOG_DIR/raw_missing_v.err" || true

echo "[9] Unsupported ALG (raw)"
printf "ALG NOPE\nV 3\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_unsupported_alg.out" 2> "$LOG_DIR/raw_unsupported_alg.err" || true

echo "[10] Invalid edge (raw)"
printf "ALG MST\nDIRECTED 0\nV 2\nE 1\nEDGE 0 5 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_invalid_edge.out" 2> "$LOG_DIR/raw_invalid_edge.err" || true

echo "[11] CRLF endings (raw)"
printf "ALG MST\r\nDIRECTED 0\r\nV 2\r\nE 1\r\nEDGE 0 1 1\r\nEND\r\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_crlf.out" 2> "$LOG_DIR/raw_crlf.err" || true

echo "[11.1] PREVIEW explicit undirected edges"
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 0\nV 4\nEDGE 0 1 2\nEDGE 1 2 2\nEDGE 2 3 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_preview_explicit.out" 2> "$LOG_DIR/raw_preview_explicit.err" || true

echo "[12] PARAM lines (raw)"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 3\nE 1\nEDGE 0 1 1\nPARAM SRC 0\nPARAM SINK 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_params.out" 2> "$LOG_DIR/raw_params.err" || true

echo "[13] Directed mismatch checks (raw)"
printf "ALG CLIQUES\nDIRECTED 1\nV 3\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_cliques_directed.out" 2> "$LOG_DIR/raw_cliques_directed.err" || true
printf "ALG MST\nDIRECTED 1\nV 3\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_mst_directed.out" 2> "$LOG_DIR/raw_mst_directed.err" || true
printf "ALG MAX_FLOW\nDIRECTED 0\nV 3\nE 0\nPARAM SRC 0\nPARAM SINK 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_maxflow_undirected_mismatch.out" 2> "$LOG_DIR/raw_maxflow_undirected_mismatch.err" || true

echo "[14] Random with WMAX < WMIN (swap)"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 5\nE 4\nWMIN 10\nWMAX 3\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_random_wswap.out" 2> "$LOG_DIR/raw_random_wswap.err" || true

echo "[15] Random graph with negative E"
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 5\nE -1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_random_E_negative.out" 2> "$LOG_DIR/raw_random_E_negative.err" || true

echo "[16] Random E too large for V"
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 6\nE 999\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_random_E_clamp.out" 2> "$LOG_DIR/raw_random_E_clamp.err" || true

echo "[17] Explicit directed MAX_FLOW (SINGLE)"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 4\nE 4\nEDGE 0 1 3\nEDGE 1 3 2\nEDGE 0 2 2\nEDGE 2 3 4\nPARAM SRC 0\nPARAM SINK 3\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_explicit_maxflow_ok.out" 2> "$LOG_DIR/raw_explicit_maxflow_ok.err" || true

echo "[18] Explicit directed SCC (SINGLE)"
printf "ALG SCC\nDIRECTED 1\nV 4\nE 4\nEDGE 0 1 1\nEDGE 1 0 1\nEDGE 2 3 1\nEDGE 3 2 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_explicit_scc_ok.out" 2> "$LOG_DIR/raw_explicit_scc_ok.err" || true

echo "[19] Explicit undirected MST (SINGLE)"
printf "ALG MST\nDIRECTED 0\nV 4\nE 3\nEDGE 0 1 1\nEDGE 1 2 2\nEDGE 2 3 3\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_explicit_mst_ok.out" 2> "$LOG_DIR/raw_explicit_mst_ok.err" || true

echo "[20] Explicit undirected CLIQUES K=3 (SINGLE)"
printf "ALG CLIQUES\nDIRECTED 0\nV 4\nE 6\nEDGE 0 1 1\nEDGE 1 2 1\nEDGE 0 2 1\nEDGE 2 3 1\nEDGE 1 3 1\nEDGE 0 3 1\nPARAM K 3\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_explicit_cliques_ok.out" 2> "$LOG_DIR/raw_explicit_cliques_ok.err" || true

echo "[21] Unknown PARAM name"
printf "ALG MST\nDIRECTED 0\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 2\nPARAM FOO 7\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_param_unknown.out" 2> "$LOG_DIR/raw_param_unknown.err" || true

echo "[22] Invalid DIRECTED numeric (treated as true)"
printf "ALG SCC\nDIRECTED 2\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_directed_2.out" 2> "$LOG_DIR/raw_directed_2.err" || true

echo "[23] Missing ALG line"
printf "DIRECTED 0\nV 3\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_missing_alg.out" 2> "$LOG_DIR/raw_missing_alg.err" || true

echo "[24] Two sequential requests on one TCP connection"
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 2\nEND\nALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 3\nE 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_two_in_one_conn.out" 2> "$LOG_DIR/raw_two_in_one_conn.err" || true

echo "[24.1] Early EOF before END (tests recv_all_lines with partial data)"
printf "ALG PREVIEW\nDIRECTED 0\nV 3\nE 2\nEDGE 0 1 1\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_partial_recv.out" 2> "$LOG_DIR/raw_partial_recv.err" || true

echo "[24.2] Client closes write early (peer_already_closed_write)"
printf "ALG PREVIEW\nDIRECTED 1\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_client_close_early.out" 2> "$LOG_DIR/raw_client_close_early.err" || true

echo "[24.3] Invalid PARAM value type"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nPARAM SRC x\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_param_invalid_value.out" 2> "$LOG_DIR/raw_param_invalid_value.err" || true

echo "[24.4] CLIQUES missing K param (forces 'Missing PARAM K')"
printf "ALG CLIQUES\nDIRECTED 0\nV 4\nE 3\nEDGE 0 1 1\nEDGE 1 2 1\nEDGE 2 3 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_missing_K.out" 2> "$LOG_DIR/raw_missing_K.err" || true

echo "[24.5] Directed mismatch for ALL (invalid combination)"
printf "ALG ALL\nDIRECTED 0\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nPARAM SRC 0\nPARAM SINK 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_all_undirected.out" 2> "$LOG_DIR/raw_all_undirected.err" || true

echo "[24.6] Random PREVIEW without WMIN/WMAX (default path)"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 3\nE 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_random_defaults.out" 2> "$LOG_DIR/raw_random_defaults.err" || true

echo "[24.7] MAX_FLOW missing SINK param"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nPARAM SRC 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_missing_sink.out" 2> "$LOG_DIR/raw_missing_sink.err" || true

echo "[24.8] EXIT command (tests graceful shutdown)"
printf "EXIT\n" | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_exit.out" 2> "$LOG_DIR/raw_exit.err" || true

echo "[24.9] Keep connection open briefly after request (peer not closed yet)"
{
  printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 2\nEND\n"
  sleep 1  
} | timeout 5s nc -w 5 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_peer_not_closed.out" 2> "$LOG_DIR/raw_peer_not_closed.err" || true

echo "[24.10] Early EOF with empty payload"
printf "" | timeout 2s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_empty_eof.out" 2> "$LOG_DIR/raw_empty_eof.err" || true



echo "[24.11] Error response without trailing newline"
printf "HELLO\nEND\n" | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > /dev/null 2>&1 || true

echo "[24.12] PREVIEW explicit directed edges"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 0\nV 3\nEDGE 0 1 5\nEDGE 2 0 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_preview_directed_explicit.out" 2> "$LOG_DIR/raw_preview_directed_explicit.err" || true

echo "[24.13] ALL directed without SRC/SINK"
printf "ALG ALL\nDIRECTED 1\nRANDOM 1\nV 4\nE 3\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_all_missing_params.out" 2> "$LOG_DIR/raw_all_missing_params.err" || true

echo "[24.14] PREVIEW seed only"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 4\nE 3\nSEED 123\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_seed_only.out" 2> "$LOG_DIR/raw_seed_only.err" || true

echo "[24.15] EDGE u v with negative w (ERR)"
printf "ALG MST\nDIRECTED 0\nV 3\nE 1\nEDGE 0 1 -5\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_edge_negative_weight.out" 2> "$LOG_DIR/raw_edge_negative_weight.err" || true

if command -v socat >/dev/null 2>&1; then
  echo "[24.16] Half-close: client write shutdown only"
  {
    printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 2\nEND\n";
  } | socat -u - TCP:127.0.0.1:$PORT,shut-down \
      > "$LOG_DIR/raw_half_close_read_open.out" 2> "$LOG_DIR/raw_half_close_read_open.err" || true
fi

echo "[24.17] Too many EDGE lines (RANDOM 0)"
printf "ALG MST\nDIRECTED 0\nRANDOM 0\nV 3\nE 1\nEDGE 0 1 1\nEDGE 1 2 1\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_too_many_edges.out" 2> "$LOG_DIR/raw_too_many_edges.err" || true

echo "[24.18] Not enough EDGE lines (RANDOM 0)"
printf "ALG MST\nDIRECTED 0\nRANDOM 0\nV 3\nE 2\nEDGE 0 1 1\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_not_enough_edges.out" 2> "$LOG_DIR/raw_not_enough_edges.err" || true

echo "[24.19] Self-loop edge should be rejected"
printf "ALG MST\nDIRECTED 0\nRANDOM 0\nV 3\nE 1\nEDGE 1 1 2\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_self_loop.out" 2> "$LOG_DIR/raw_self_loop.err" || true

echo "[24.20] EDGE with non-integer weight"
printf "ALG MST\nDIRECTED 0\nRANDOM 0\nV 3\nE 1\nEDGE 0 1 two\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_edge_bad_weight.out" 2> "$LOG_DIR/raw_edge_bad_weight.err" || true

echo "[24.21] Vertex index == V (out-of-range high)"
printf "ALG MST\nDIRECTED 0\nRANDOM 0\nV 3\nE 1\nEDGE 0 3 1\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_vertex_eq_v.out" 2> "$LOG_DIR/raw_vertex_eq_v.err" || true

echo "[24.22] WMIN == 0 -> invalid"
printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 2\nWMIN 0\nWMAX 5\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_wmin_zero.out" 2> "$LOG_DIR/raw_wmin_zero.err" || true

echo "[24.23] SEED negative"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 3\nE 2\nSEED -42\nEND\n" \
 | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
 > "$LOG_DIR/raw_seed_negative.out" 2> "$LOG_DIR/raw_seed_negative.err" || true


# ======================  BlockingQueue header coverage  ==============
echo "[25] BlockingQueue header unit test"
cat > "$LOG_DIR/bq_test.cpp" <<'EOF'
#include <thread>
#include <cassert>
#include <iostream>
#include "../include/blocking_queue.hpp"

int main() {
  BlockingQueue<int> q(2);
  assert(q.empty());
  assert(!q.closed());
  q.push(1);
  q.push(2);
  int x;
  bool got = q.try_pop(x);
  if (got) { /* ok */ }

  q.push(int{3});
  std::thread consumer([&](){
    int y;
    while (q.pop(y)) { /* drain until close */ }
  });

  (void)q.size();
  (void)q.empty();
  (void)q.closed();

  q.close();
  consumer.join();
  bool ok = q.push(42);
  if (ok) std::cerr << "push after close should fail\n";
  return 0;
}
EOF



# ---------- SERVER paths not yet covered well ----------

echo "[27] Invalid EDGE weight (<=0) -> ERR"
printf "ALG MST\nDIRECTED 0\nV 3\nE 1\nEDGE 0 1 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_edge_weight_zero.out" 2> "$LOG_DIR/raw_edge_weight_zero.err" || true

echo "[28] MAX_FLOW invalid (SRC==SINK)"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 4\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nPARAM SRC 1\nPARAM SINK 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_maxflow_src_eq_sink.out" 2> "$LOG_DIR/raw_maxflow_src_eq_sink.err" || true

echo "[29] CLIQUES invalid K (<2)"
printf "ALG CLIQUES\nDIRECTED 0\nV 4\nE 3\nEDGE 0 1 1\nEDGE 1 2 1\nEDGE 2 3 1\nPARAM K 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_cliques_k_lt_2.out" 2> "$LOG_DIR/raw_cliques_k_lt_2.err" || true

echo "[30] CLIQUES invalid K (>V)"
printf "ALG CLIQUES\nDIRECTED 0\nV 4\nE 6\nEDGE 0 1 1\nEDGE 0 2 1\nEDGE 0 3 1\nEDGE 1 2 1\nEDGE 1 3 1\nEDGE 2 3 1\nPARAM K 5\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_cliques_k_gt_v.out" 2> "$LOG_DIR/raw_cliques_k_gt_v.err" || true

echo "[31] DIRECTED -1 "
printf "ALG SCC\nDIRECTED -1\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_directed_minus1.out" 2> "$LOG_DIR/raw_directed_minus1.err" || true

echo "[32] PREVIEW random with E=0 (count header w/o edges)"
printf "ALG PREVIEW\nDIRECTED 1\nRANDOM 1\nV 5\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_preview_e_zero.out" 2> "$LOG_DIR/raw_preview_e_zero.err" || true

if [ "${SKIP_HEAVY:-0}" = "1" ]; then
  echo "[33] Skipped heavy case (V=70000) due to SKIP_HEAVY=1"
else
    echo "[33] Client overflow path for maxE (V=15000, undirected)"
  cat > "$LOG_DIR/input_client_overflow_v.txt" <<'EOF'
1
0
15000
0
1
1
-1
-1
0
0
EOF
fi




# ---------- CLIENT prompt_int extra tricky inputs ----------

echo "[34] Client prompt_int: '-a' when negatives allowed -> Invalid number"
cat > "$LOG_DIR/input_client_minus_alpha.txt" <<'EOF'
1
1
3
2
1
1
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/input_client_minus_alpha.txt" \
  "$LOG_DIR/client_minus_alpha.out" "$LOG_DIR/client_minus_alpha.err"

echo "[35] Client prompt_int: whitespace around numbers + skip run-all"
cat > "$LOG_DIR/input_client_whitespace.txt" <<'EOF'
1
 0 
 6 
 3 
 1 
 3 
 -1 
 -1 
 0 
0
EOF
run_with_input "$LOG_DIR/input_client_whitespace.txt" \
  "$LOG_DIR/client_whitespace.out" "$LOG_DIR/client_whitespace.err"

# ---------- Signals & EINTR coverage ----------

sleep 0.2

echo "[36] Send SIGUSR1 while server is idle (cover handler + EINTR path)"
kill -USR1 "$SERVER_PID" 2>/dev/null || true
sleep 0.1
kill -USR1 "$SERVER_PID" 2>/dev/null || true
sleep 0.1

printf "ALG PREVIEW\nDIRECTED 0\nRANDOM 1\nV 3\nE 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_post_usr1_ping.out" 2> "$LOG_DIR/raw_post_usr1_ping.err" || true




echo "[38] MAX_FLOW missing SRC param (only SINK provided)"
printf "ALG MAX_FLOW\nDIRECTED 1\nV 3\nE 2\nEDGE 0 1 1\nEDGE 1 2 1\nPARAM SINK 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_missing_src.out" 2> "$LOG_DIR/raw_missing_src.err" || true

echo "[39] EDGE without weight (default should be 1)"
printf "ALG MST\nDIRECTED 0\nV 3\nE 2\nEDGE 0 1\nEDGE 1 2 2\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_edge_default_weight.out" 2> "$LOG_DIR/raw_edge_default_weight.err" || true

echo "[40] EDGE with negative vertex index"
printf "ALG MST\nDIRECTED 0\nV 3\nE 1\nEDGE -1 2 1\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_edge_negative_vertex.out" 2> "$LOG_DIR/raw_edge_negative_vertex.err" || true

echo "[41] DIRECTED 'foo' "
printf "ALG PREVIEW\nDIRECTED foo\nV 3\nE 0\nEND\n" \
  | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_directed_text.out" 2> "$LOG_DIR/raw_directed_text.err" || true


echo "[42] Client: refuse ALL after PREVIEW (yn != '1')"
cat > "$LOG_DIR/input_client_refuse_all.txt" <<'EOF'
1
0
5
4
1
7
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/input_client_refuse_all.txt" \
  "$LOG_DIR/client_refuse_all.out" "$LOG_DIR/client_refuse_all.err"

sleep 0.3


g++ -std=c++17 -O0 -g --coverage -pthread -I.. -I../apps -I../include \
  -o "$LOG_DIR/bq_test" "$LOG_DIR/bq_test.cpp"
"$LOG_DIR/bq_test" > "$LOG_DIR/bq_test.out" 2> "$LOG_DIR/bq_test.err" || true

echo "[C0] Client: PREVIEW then refuse ALL then EXIT"
cat > "$LOG_DIR/input_client_quick_preview.txt" <<'EOF'
1
0
4
3
1
2
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/input_client_quick_preview.txt" \
  "$LOG_DIR/client_quick_preview.out" "$LOG_DIR/client_quick_preview.err"

echo "[C0b] Client: PREVIEW then ALL then EXIT"
cat > "$LOG_DIR/input_client_preview_all.txt" <<'EOF'
1
0
5
4
1
3
-1
-1
1
exit
EOF
run_with_input "$LOG_DIR/input_client_preview_all.txt" \
  "$LOG_DIR/client_preview_all.out" "$LOG_DIR/client_preview_all.err"

echo "[C1] Client menu: SINGLE MAX_FLOW"
cat > "$LOG_DIR/input_client_single_maxflow.txt" <<'EOF'
2     
1      # Directed
4      # V
4      # E
0 1 3  # EDGE
1 3 2
0 2 2
2 3 4
0      # SRC
3      # SINK
0      # Back to main
0      # Exit
EOF
run_with_input "$LOG_DIR/input_client_single_maxflow.txt" \
  "$LOG_DIR/client_single_maxflow.out" "$LOG_DIR/client_single_maxflow.err"

echo "[C2] Client menu: SINGLE SCC"
cat > "$LOG_DIR/input_client_single_scc.txt" <<'EOF'
2
1
4
4
0 1 1
1 0 1
2 3 1
3 2 1
0
0
EOF
run_with_input "$LOG_DIR/input_client_single_scc.txt" \
  "$LOG_DIR/client_single_scc.out" "$LOG_DIR/client_single_scc.err"

echo "[C3] Client menu: SINGLE MST"
cat > "$LOG_DIR/input_client_single_mst.txt" <<'EOF'
2
0
4
3
0 1 1
1 2 2
2 3 3
0
0
EOF
run_with_input "$LOG_DIR/input_client_single_mst.txt" \
  "$LOG_DIR/client_single_mst.out" "$LOG_DIR/client_single_mst.err"

echo "[C4] Client menu: SINGLE CLIQUES (K=3)"
cat > "$LOG_DIR/input_client_single_cliques.txt" <<'EOF'
2
0
4
6
0 1 1
1 2 1
0 2 1
2 3 1
1 3 1
0 3 1
3      # K
0
0
EOF
run_with_input "$LOG_DIR/input_client_single_cliques.txt" \
  "$LOG_DIR/client_single_cliques.out" "$LOG_DIR/client_single_cliques.err"

echo "[C5] Client: invalid menu choice then valid"
cat > "$LOG_DIR/input_client_bad_menu.txt" <<'EOF'
9
1
0
6
7
1
5
-1
3
0
0
EOF
run_with_input "$LOG_DIR/input_client_bad_menu.txt" \
  "$LOG_DIR/client_bad_menu.out" "$LOG_DIR/client_bad_menu.err"

echo "[C6] Client: FULL ALL happy path"
cat > "$LOG_DIR/input_client_all_happy.txt" <<'EOF'
1      # mode=ALL
0      # directed? 0=undirected
5      # V
4      # E
1      # WMIN
5      # WMAX
-1     # SRC skip
-1     # K skip
1      # Run ALL after preview
exit   # quit after ALL
EOF
run_with_input "$LOG_DIR/input_client_all_happy.txt" \
  "$LOG_DIR/client_all_happy.out" "$LOG_DIR/client_all_happy.err"

echo "[C7] Client: PREVIEW only then exit"
cat > "$LOG_DIR/input_client_preview_only2.txt" <<'EOF'
1      # mode=ALL
1      # directed
4      # V
3      # E
2      # WMIN
3      # WMAX
0      # SRC=0 (max flow relevant)
1      # SINK=1 (!=SRC)
-1     # K skip
0      # Do NOT run ALL after preview
0      # back to main -> Exit
EOF
run_with_input "$LOG_DIR/input_client_preview_only2.txt" \
  "$LOG_DIR/client_preview_only2.out" "$LOG_DIR/client_preview_only2.err"

echo "[C8] Client: prompt_int errors (Invalid number + Out of range)"
cat > "$LOG_DIR/input_client_prompt_errors.txt" <<'EOF'
1      # mode=ALL
0      # directed
a      # V -> Invalid number
1      # V -> Out of range (min 2)
2      # V ok
5      # E -> Out of range 
0      # E ok
0      # WMIN -> Out of range (min 1)
1      # WMIN ok
0      # WMAX -> Out of range (min=WMIN=1)
1      # WMAX ok
-      # SRC -> Invalid number 
-1     # SRC skip
-1     # K skip
0      # Do NOT run ALL after preview
0      # Exit
EOF
run_with_input "$LOG_DIR/input_client_prompt_errors.txt" \
  "$LOG_DIR/client_prompt_errors.out" "$LOG_DIR/client_prompt_errors.err"

echo "[C9] Client: SINK equals SRC loop"
cat > "$LOG_DIR/input_client_sink_eq_src.txt" <<'EOF'
1      # mode=ALL
1      # directed
4      # V
3      # E
1      # WMIN
2      # WMAX
2      # SRC=2
2      # SINK=2 
3      # SINK=3 -> ok
-1     # K skip
0      # Do NOT run ALL after preview
0      # Exit
EOF
run_with_input "$LOG_DIR/input_client_sink_eq_src.txt" \
  "$LOG_DIR/client_sink_eq_src.out" "$LOG_DIR/client_sink_eq_src.err"

echo "[C10] Client: run two rounds then exit with menu 0"
cat > "$LOG_DIR/input_client_two_rounds.txt" <<'EOF'
1
0
4
3
1
2
-1
-1
1
 
1
1
3
2
1
1
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/input_client_two_rounds.txt" \
  "$LOG_DIR/client_two_rounds.out" "$LOG_DIR/client_two_rounds.err"

echo "[1b.5] Mid-suite restart"
restart_server

echo "[CX1] Client: PREVIEW -> ALL -> exit (clean)"
cat > "$LOG_DIR/in_client_preview_all_ok.txt" <<'EOF'
1
0
5
4
1
3
-1
-1
1
exit
EOF
run_with_input "$LOG_DIR/in_client_preview_all_ok.txt" \
  "$LOG_DIR/out_client_preview_all_ok.out" "$LOG_DIR/out_client_preview_all_ok.err"

echo "[CX2] Client: SINK==SRC loop then refuse ALL (clean)"
cat > "$LOG_DIR/in_client_sink_eq_src_loop.txt" <<'EOF'
1
1
4
3
1
2
2
2
3
-1
0
0
EOF
run_with_input "$LOG_DIR/in_client_sink_eq_src_loop.txt" \
  "$LOG_DIR/out_client_sink_eq_src_loop.out" "$LOG_DIR/out_client_sink_eq_src_loop.err"

echo "[CX3] Client: prompt_int errors (Invalid/Out-of-range) clean"
cat > "$LOG_DIR/in_client_prompt_errors_clean.txt" <<'EOF'
1
0
a
1
2
5
0
0
1
0
1
-
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/in_client_prompt_errors_clean.txt" \
  "$LOG_DIR/out_client_prompt_errors_clean.out" "$LOG_DIR/out_client_prompt_errors_clean.err"

if [ "${SKIP_HEAVY:-0}" = "1" ]; then
  echo "[CX4] Skipped heavy case (V=70000) due to SKIP_HEAVY=1"
else
    echo "[CX4] Client: overflow path for maxE (clean)"
  cat > "$LOG_DIR/in_client_overflow_maxE.txt" <<'EOF'
1
0
15000
0
1
1
-1
-1
0
0
EOF
  run_with_input "$LOG_DIR/in_client_overflow_maxE.txt" \
    "$LOG_DIR/out_client_overflow_maxE.out" "$LOG_DIR/out_client_overflow_maxE.err"
fi

echo "[CX5] Client: whitespace around numbers + PREVIEW only (clean)"
cat > "$LOG_DIR/in_client_whitespace_clean.txt" <<'EOF'
1
 0 
 6 
 3 
 1 
 3 
 -1 
 -1 
 0 
0
EOF
run_with_input "$LOG_DIR/in_client_whitespace_clean.txt" \
  "$LOG_DIR/out_client_whitespace_clean.out" "$LOG_DIR/out_client_whitespace_clean.err"

echo "[CX6] Client: negative digits branch in prompt_int"
cat > "$LOG_DIR/in_client_negative_digits.txt" <<'EOF'
1
1
4
2
1
2
-5
-1
-1
0
0
EOF
run_with_input "$LOG_DIR/in_client_negative_digits.txt" \
  "$LOG_DIR/out_client_negative_digits.out" "$LOG_DIR/out_client_negative_digits.err"


flush_coverage


echo "[26] Graceful shutdown "
printf "SHUTDOWN\n" | timeout 5s nc $NC_CLOSE_OPT -w 2 127.0.0.1 "$PORT" \
  > "$LOG_DIR/raw_shutdown.out" 2> "$LOG_DIR/raw_shutdown.err" || true
sleep 0.3

flush_coverage

echo "[37] Terminate with SIGTERM (covers sigterm_handler path)"
flush_coverage
kill -TERM "$SERVER_PID" 2>/dev/null || true
wait "$SERVER_PID" 2>/dev/null || true
sleep 0.3



echo "=== GCOV (post-run) ==="
find . -name '*.gcda' | sed 's/^/  /'

# Compute aggregate totals without generating .gcov files (faster)
if ls ./*.gcda >/dev/null 2>&1; then
  gcov -b -n ./*.gcda 2>/dev/null | awk '
    BEGIN { lh=0; lt=0; bh=0; bt=0 }
    /Lines executed:/ {
      if (match($0, /Lines executed: *([0-9.]+)% of *([0-9]+)/, a)) {
        lt += a[2];
        lh += int(a[1]*a[2]/100 + 0.5);
      }
    }
    /Taken at least once:/ {
      if (match($0, /Taken at least once: *([0-9.]+)% of *([0-9]+)/, c)) {
        bt += c[2];
        bh += int(c[1]*c[2]/100 + 0.5);
      }
    }
    END {
      lp = (lt ? 100.0*lh/lt : 0);
      bp = (bt ? 100.0*bh/bt : 0);
      printf("Total Line Coverage: %d/%d = %.2f%%\n", lh, lt, lp);
      printf("Total Branch Coverage: %d/%d = %.2f%%\n", bh, bt, bp);
    }
  '
fi


echo " All test runs completed."
