#!/bin/bash
set -euo pipefail

mkdir -p build

# ---------- Utilities ----------

graceful_stop_server() {
  local pid="$1"
  if kill -0 "$pid" 2>/dev/null; then sleep 0.3; fi
  if kill -0 "$pid" 2>/dev/null; then kill "$pid" 2>/dev/null || true; sleep 0.2; fi
  if kill -0 "$pid" 2>/dev/null; then kill -9 "$pid" 2>/dev/null || true; fi
}

send_raw() {
  # send_raw "payload-string"
  local payload="$1"
  local outfile="$2"
  local errfile="$3"

  if command -v nc >/dev/null 2>&1; then
    printf "%s" "$payload" | nc -q 1 127.0.0.1 8080 >"$outfile" 2>"$errfile" || true
  else
    # Fallback: /dev/tcp (Bash)
    exec 3<>/dev/tcp/127.0.0.1/8080 || true
    if [ -e /proc/$$/fd/3 ]; then
      printf "%s" "$payload" >&3
      timeout 2s cat <&3 >"$outfile" 2>"$errfile" || true
      exec 3>&- 3<&-
    else
      echo "raw socket open failed" >"$errfile"
    fi
  fi
}

# ---------- Main flow ----------

echo "[1] Running server..."
./server > build/server.out 2> build/server.err &
SERVER_PID=$!
sleep 0.5

echo "[2] Complex invalid + valid input"
timeout 6s ./client > build/client_complex.out 2> build/client_complex.err <<'EOF'
-2
a
!
3
4
a
3
1 1
1 -2
1 5
1 2
1 0
2 0
n
EOF

echo "[3] One edge (not Eulerian)"
timeout 6s ./client > build/client_one_edge.out 2> build/client_one_edge.err <<'EOF'
3
1
1 2
n
EOF

echo "[4] Valid Eulerian circuit"
timeout 6s ./client > build/client_valid_euler.out 2> build/client_valid_euler.err <<'EOF'
3
2
0 1
1 2
n
EOF

echo "[5] Exit in the middle of edge input"
timeout 6s ./client > build/client_exit_mid_edges.out 2> build/client_exit_mid_edges.err <<'EOF'
3
2
0 1
exit
EOF

echo "[6] Invalid answer in again loop"
timeout 6s ./client > build/client_invalid_again.out 2> build/client_invalid_again.err <<'EOF'
3
0
maybe
n
EOF

echo "[7] Edges larger than max"
timeout 6s ./client > build/client_edges_gt_max.out 2> build/client_edges_gt_max.err <<'EOF'
3
5
2
0 1
1 2
n
EOF

echo "[8] Empty graph (E=0)"
timeout 6s ./client > build/client_empty_graph.out 2> build/client_empty_graph.err <<'EOF'
3
0
n
EOF

echo "[9] V too small (trigger V<=1)"
timeout 6s ./client > build/client_v_too_small.out 2> build/client_v_too_small.err <<'EOF'
1
3
0
n
EOF

echo "[10] Edge with no space"
timeout 6s ./client > build/client_edge_no_space.out 2> build/client_edge_no_space.err <<'EOF'
3
1
01
0 1
n
EOF

echo "[11] Non-numeric edge (stoi exception)"
timeout 6s ./client > build/client_edge_non_numeric.out 2> build/client_edge_non_numeric.err <<'EOF'
3
1
x y
0 1
n
EOF

echo "[12] Duplicate edge"
timeout 6s ./client > build/client_edge_duplicate.out 2> build/client_edge_duplicate.err <<'EOF'
3
2
0 1
1 0
2 0
n
EOF

echo "[13] Again loop – user chooses 'y'"
timeout 6s ./client > build/client_again_y.out 2> build/client_again_y.err <<'EOF'
3
0
y
3
0
n
EOF

echo "[14] Again loop – invalid then 'n'"
timeout 6s ./client > build/client_again_invalid.out 2> build/client_again_invalid.err <<'EOF'
3
0
bla
n
EOF

echo "[15] Server-side: V <= 0 (invalid vertices)"
timeout 6s ./client > build/server_V_le_0.out 2> build/server_V_le_0.err <<'EOF'
0
3
0
n
EOF

# --------- Server-only branches via RAW socket (without client validation) ---------

echo "[15b] RAW: E < 0 to hit 'edges cannot be negative'"
send_raw "3 -1\n" build/raw_E_negative.out build/raw_E_negative.err

echo "[15c] RAW: invalid edge (out of range) to hit 'Invalid edge (...)'"
send_raw "3 1\n5 0\n" build/raw_edge_oob.out build/raw_edge_oob.err

# --------- Clean shutdown of the main server ---------
echo "[16] EXIT_CLIENT to close main server (clean coverage flush)"
timeout 6s ./client > /dev/null 2>&1 <<'EOF'
2
0
n
EOF
sleep 0.4
graceful_stop_server "$SERVER_PID" || true

echo "[17] Connection error (server down)"
timeout 3s ./client > build/client_conn_fail1.out 2> build/client_conn_fail1.err || true

echo "[19] Connection error (server already down)"
timeout 3s ./client > build/client_conn_fail2.out 2> build/client_conn_fail2.err <<'EOF' || true
3
0
n
EOF

echo "[20] Again loop – invalid then 'n' (extra)"
timeout 6s ./client > build/client_again_invalid_prompt.out 2> build/client_again_invalid_prompt.err <<'EOF'
3
0
koko
n
EOF

echo "[21] Server timeout (no clients for 30s)"
./server > build/server_timeout.out 2> build/server_timeout.err &
SERVER_PID=$!
timeout 40s tail --pid="$SERVER_PID" -f /dev/null 2>/dev/null || true

echo "[22] Done."
