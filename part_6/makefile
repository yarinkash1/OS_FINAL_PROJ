CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -g --coverage -fprofile-arcs -ftest-coverage
LDFLAGS = --coverage -pthread

SRCS_SERVER = server.cpp
SRCS_CLIENT = client.cpp
SRCS_UTIL   = ../part_1/graph_impl.cpp ../part_2/euler_circle.cpp

OBJS_SERVER = server.o graph_impl.o euler_circle.o
OBJS_CLIENT = client.o

BIN_SERVER = server
BIN_CLIENT = client

.PHONY: all clean gcov valgrind memcheck callgrind helgrind

all: $(BIN_SERVER) $(BIN_CLIENT)

server.o: server.cpp server.hpp
	$(CXX) $(CXXFLAGS) -c server.cpp -o $@

client.o: client.cpp client.hpp
	$(CXX) $(CXXFLAGS) -c client.cpp -o $@

graph_impl.o: ../part_1/graph_impl.cpp
	$(CXX) -std=c++17 -Wall -Wextra -O0 -g -c ../part_1/graph_impl.cpp -o $@

euler_circle.o: ../part_2/euler_circle.cpp
	$(CXX) -std=c++17 -Wall -Wextra -O0 -g -c ../part_2/euler_circle.cpp -o $@


$(BIN_SERVER): $(OBJS_SERVER)
	$(CXX) $(OBJS_SERVER) $(LDFLAGS) -o $@

$(BIN_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(OBJS_CLIENT) $(LDFLAGS) -o $@

# ===== Simplified Coverage (single total target) =====
gcov: clean all
	@FAST_TIMEOUT=1 ./run_tests.sh >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_SOCKET_ERROR=1 ./server >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_SETSOCKOPT_ERROR=1 ./server >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_BIND_ERROR=1 ./server >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_LISTEN_ERROR=1 ./server >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_SELECT_ERROR=1 ./server >/dev/null 2>&1 || true
	@FAST_TIMEOUT=1 SUPPRESS_EXIT=1 COV_FORCE_ACCEPT_ERROR=1 ./server >/dev/null 2>&1 || true
	@rm -f server.cpp.gcov client.cpp.gcov || true
	@gcov -b -c server.cpp -o . >/dev/null 2>&1 || true
	@gcov -b -c client.cpp -o . >/dev/null 2>&1 || true
	@bash tools/coverage_totals_quiet.sh


# ===== Valgrind =====


valgrind-mem: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Memcheck =="
	./server > /dev/null 2>&1 &
	sleep 0.2
	printf "3\n1\n1 2\nn\n" | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client

valgrind-cg: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Callgrind =="
	./server > /dev/null 2>&1 &
	sleep 0.2
	printf "4\n4\n0 1\n1 2\n2 3\n3 0\nn\n" | valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./client
	@echo "Use kcachegrind to view: callgrind.out"

valgrind-hel: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Helgrind =="
	./server > /dev/null 2>&1 &
	sleep 0.2
	printf "3\n1\n1 2\nn\n" | valgrind --tool=helgrind ./client

# ===== Clean =====
clean:
	rm -f $(BIN_SERVER) $(BIN_CLIENT) *.o *.gcno *.gcda *.gcov \
	      server-*.gcno server-*.gcda client-*.gcno client-*.gcda \
	      callgrind.out* cachegrind.out* gmon.out gprof_report.txt
	rm -rf build cov_* server_err server_norm
