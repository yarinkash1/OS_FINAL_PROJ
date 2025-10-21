CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pthread -O0 -g -fprofile-arcs -ftest-coverage
LDFLAGS=--coverage

# Paths
ROOT=..
PART1=$(ROOT)/../part_1
PART7=$(ROOT)/../part_7
APPS=$(ROOT)/apps
INC=$(ROOT)/include

INCLUDES=-I$(ROOT) -I$(APPS) -I$(INC) -I$(PART1) -I$(PART7) -I$(PART7)/algorithms -I$(PART7)/strategy_factory
ALGO_SRCS=$(wildcard $(PART7)/algorithms/*.cpp)
ALGO_OBJS=$(patsubst %.cpp,%.o,$(notdir $(ALGO_SRCS)))
RAND_SRC=$(ROOT)/../part_8/include/random_graph.cpp

BIN_SERVER=server
BIN_CLIENT=client

.PHONY: all clean gcov gcov-quick valgrind memcheck callgrind helgrind

# ===== Build =====
all: $(BIN_SERVER) $(BIN_CLIENT)

server.o: $(APPS)/server.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

AlgorithmFactory.o: $(PART7)/strategy_factory/AlgorithmFactory.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

graph_impl.o: $(PART1)/graph_impl.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

random_graph.o: $(RAND_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(ALGO_OBJS): %.o: $(PART7)/algorithms/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# link
$(BIN_SERVER): server.o AlgorithmFactory.o graph_impl.o random_graph.o $(ALGO_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_CLIENT): $(APPS)/client.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

gcov: $(BIN_SERVER) $(BIN_CLIENT)
	# Full suite with heavy tests skipped; for fastest results use `make gcov-quick`
	SKIP_HEAVY=1 ./run_tests.sh
	@echo "=== GCOV REPORTS ==="
	@echo "--- Server ---";  gcov -b -n $(APPS)/server.cpp  -o . | grep -E "File|Lines executed|Branches executed|Taken at least once" || true
	@echo "--- Client ---";  gcov -b -n $(APPS)/client.cpp  -o . | grep -E "File|Lines executed|Branches executed|Taken at least once" || true
	@echo "--- Totals (all objects) ---"; \
	if ls ./*.gcda >/dev/null 2>&1; then \
	  gcov -b -n ./*.gcda 2>/dev/null | awk 'BEGIN { lh=0; lt=0; bh=0; bt=0 } /Lines executed:/ { if (match($$0, /Lines executed: *([0-9.]+)% of *([0-9]+)/, a)) { lt += a[2]; lh += int(a[1]*a[2]/100 + 0.5); } } /Taken at least once:/ { if (match($$0, /Taken at least once: *([0-9.]+)% of *([0-9]+)/, c)) { bt += c[2]; bh += int(c[1]*c[2]/100 + 0.5); } } END { lp = (lt ? 100.0*lh/lt : 0); bp = (bt ? 100.0*bh/bt : 0); printf("Total Line Coverage: %d/%d = %.2f%%\n", lh, lt, lp); printf("Total Branch Coverage: %d/%d = %.2f%%\n", bh, bt, bp); }'; \
	else \
	  echo "No .gcda files found"; \
	fi

# Ultra-fast smoke + coverage totals (no .gcov generation)
gcov-quick: $(BIN_SERVER) $(BIN_CLIENT)
	SKIP_HEAVY=1 QUICK=1 ./run_tests.sh

valgrind-mem: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Memcheck =="
	./$(BIN_SERVER) > /dev/null 2>&1 &
	sleep 0.3
	printf "0\n" | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(BIN_CLIENT)

valgrind-cg: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Callgrind =="
	./$(BIN_SERVER) > /dev/null 2>&1 &
	sleep 0.3
	printf "0\n" | valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./$(BIN_CLIENT)
	@echo "Use kcachegrind to view: callgrind.out"

valgrind-hel: $(BIN_SERVER) $(BIN_CLIENT)
	@echo "== Valgrind Helgrind =="
	./$(BIN_SERVER) > /dev/null 2>&1 &
	sleep 0.3
	printf "0\n" | valgrind --tool=helgrind ./$(BIN_CLIENT)

# ===== Clean =====
clean:
	rm -f $(BIN_SERVER) $(BIN_CLIENT) *.o \
	      *.gcno *.gcda *.gcov \
	      callgrind.out* cachegrind.out* gmon.out
