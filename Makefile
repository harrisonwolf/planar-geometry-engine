CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
OBJ = main.o die.o point.o triangle.o polygon_new.o line.o helper.o choice.o ear_clipping_triangulation.o

all: main

# Debug build: enables #ifdef DEBUG blocks and symbols
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean main

main: $(OBJ) logger.h
	$(CXX) $(CXXFLAGS) -o main $(OBJ)

main.o: main.cc
	$(CXX) $(CXXFLAGS) -c main.cc

die.o: die.cc die.h
	$(CXX) $(CXXFLAGS) -c die.cc

point.o: point.cc point.h
	$(CXX) $(CXXFLAGS) -c point.cc

triangle.o: triangle.cc triangle.h
	$(CXX) $(CXXFLAGS) -c triangle.cc

polygon_new.o: polygon_new.cc polygon_new.h
	$(CXX) $(CXXFLAGS) -c polygon_new.cc

line.o: line.cc line.h
	$(CXX) $(CXXFLAGS) -c line.cc

helper.o: helper.cc helper.h
	$(CXX) $(CXXFLAGS) -c helper.cc

choice.o: choice.cc choice.h
	$(CXX) $(CXXFLAGS) -c choice.cc

ear_clipping_triangulation.o: ear_clipping_triangulation.cc ear_clipping_triangulation.h logger.h
	$(CXX) $(CXXFLAGS) -c ear_clipping_triangulation.cc

clean:
	rm -f *.o main

