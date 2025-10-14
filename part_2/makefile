CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage
LDFLAGS  = -fprofile-arcs -ftest-coverage
INCLUDES = -I../part_1

all: euler

euler: main.o euler_circle.o ../part_1/graph_impl.o
	$(CXX) $(LDFLAGS) -o $@ $^

main.o: main.cpp euler_circle.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

euler_circle.o: euler_circle.cpp euler_circle.hpp ../part_1/graph_impl.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

../part_1/graph_impl.o: ../part_1/graph_impl.cpp ../part_1/graph_impl.hpp
	$(CXX) $(CXXFLAGS) -c ../part_1/graph_impl.cpp -o ../part_1/graph_impl.o

# ---- Coverage (gcov) ----
gcov: all
	./euler
	gcov -b -c -o . euler_circle.cpp main.cpp


# ---- Valgrind tools ----
valgrind-mem: all
	valgrind --leak-check=full --show-leak-kinds=all ./euler

valgrind-hel: all
	valgrind --tool=helgrind ./euler	

valgrind-cg: all
	valgrind --tool=callgrind ./euler

clean:
	rm -f euler *.o ../part_1/graph_impl.o *.gcno *.gcda *.gcov callgrind.out.*

.PHONY: all clean gcov euler valgrind-mem valgrind-hel valgrind-cg
