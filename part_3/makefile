CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage
LDFLAGS  = -fprofile-arcs -ftest-coverage
INCLUDES = -I../part_1 -I../part_2

all: main

main: main.o random_graph.o ../part_1/graph_impl.o ../part_2/euler_circle.o
	$(CXX) $(LDFLAGS) -o $@ $^

main.o: main.cpp random_graph.hpp ../part_1/graph_impl.hpp ../part_2/euler_circle.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

random_graph.o: random_graph.cpp random_graph.hpp ../part_1/graph_impl.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

../part_1/graph_impl.o: ../part_1/graph_impl.cpp ../part_1/graph_impl.hpp
	$(CXX) $(CXXFLAGS) -c ../part_1/graph_impl.cpp -o ../part_1/graph_impl.o

../part_2/euler_circle.o: ../part_2/euler_circle.cpp ../part_2/euler_circle.hpp ../part_1/graph_impl.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c ../part_2/euler_circle.cpp -o ../part_2/euler_circle.o

# ---- GCOV: run scenarios and generate report ----
gcov: all
	./run_gcov_tests.sh
	gcov -b -c -o . main.cpp random_graph.cpp


# ---- Valgrind ----
valgrind-mem: all
	valgrind --leak-check=full --show-leak-kinds=all ./main -v 4 -e 4 -s 42

valgrind-hel: all
	valgrind --tool=helgrind ./main -v 4 -e 4 -s 42

valgrind-cg: all
	valgrind --tool=callgrind ./main -v 4 -e 4 -s 42

clean:
	rm -f main *.o ../part_1/graph_impl.o ../part_2/euler_circle.o *.gcno *.gcda *.gcov callgrind.out.*

.PHONY: all clean gcov valgrind-mem valgrind-hel valgrind-cg