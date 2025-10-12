CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -O0 -g --coverage
LDFLAGS   = --coverage
BIN       = main

SRC       = main.cpp \
             ../part_1/graph_impl.cpp \
             ../part_2/euler_circle.cpp \
             ../part_3/random_graph.cpp

OBJ       = main.o \
             ../part_1/graph_impl.o \
             ../part_2/euler_circle.o \
             ../part_3/random_graph.o

ARGS_RAND = -v $(shell echo $$((RANDOM % 50 + 2))) \
             -e $(shell echo $$((RANDOM % 100 + 1))) \
             -s $(shell date +%s)

# ==== Targets ====

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
../part_1/graph_impl.o: ../part_1/graph_impl.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
../part_2/euler_circle.o: ../part_2/euler_circle.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
../part_3/random_graph.o: ../part_3/random_graph.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==== Coverage (gcov) ====
gcov: all
	./run_tests.sh || true
	@echo "=== GCOV Reports ==="
	gcov -b -c -o .            main.cpp             || true
	gcov -b -c -o ../part_1    ../part_1/graph_impl.cpp   || true
	gcov -b -c -o ../part_2    ../part_2/euler_circle.cpp || true
	gcov -b -c -o ../part_3    ../part_3/random_graph.cpp || true

# ==== Valgrind ====
valgrind-mem: $(BIN)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --errors-for-leak-kinds=all --error-exitcode=1 \
	         ./$(BIN) $(ARGS_RAND)

valgrind-hel: $(BIN)
	valgrind --tool=helgrind --history-level=full --error-exitcode=1 \
	         ./$(BIN) $(ARGS_RAND)

valgrind-cg: $(BIN)
	valgrind --tool=callgrind --callgrind-out-file=callgrind.out.$(BIN) \
	         ./$(BIN) -v 10 -e 15 -s 42
	@echo "Open with: kcachegrind callgrind.out.$(BIN)"
	valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.out.$(BIN) \
	         ./$(BIN) -v 10 -e 15 -s 42
	@echo "Annotate with: cg_annotate cachegrind.out.$(BIN)"

valgrind: memcheck   

# ==== gprof ====
main_gprof: $(SRC)
	$(CXX) $(CXXFLAGS) -pg -o main_gprof $(SRC)
gprof: main_gprof
	./main_gprof -v 10 -e 15 -s 42
	gprof ./main_gprof gmon.out > gprof_report.txt

# ==== Clean ====
clean:
	rm -f $(BIN) main_gprof $(OBJ) \
	      *.gcno *.gcda *.gcov \
	      ../part_1/*.gcno ../part_1/*.gcda \
	      ../part_2/*.gcno ../part_2/*.gcda \
	      ../part_3/*.gcno ../part_3/*.gcda \
	      callgrind.out.* cachegrind.out.* \
	      gprof_report.txt gmon.out

.PHONY: all gcov memcheck helgrind callgrind cachegrind cg valgrind gprof clean