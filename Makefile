CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
EXE = main rand_poly_gen_driver
OBJ = die.o point.o triangle.o polygon_new.o line.o helper.o choice.o ear_clipping_triangulation.o random_polygon_generator.o delaunay.o

all: main rand_poly_gen_driver

# Debug build: enables #ifdef DEBUG blocks and symbols
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean all
# need to figure out how to make it clean only if last make was not debug build

main: main.o $(OBJ) logger.h
	$(CXX) $(CXXFLAGS) -o main main.o $(OBJ)

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

random_polygon_generator.o: random_polygon_generator.cc random_polygon_generator.h polygon_new.h logger.h
	$(CXX) $(CXXFLAGS) -c random_polygon_generator.cc 

delaunay.o: delaunay.cc delaunay.h triangle.h
	$(CXX) $(CXXFLAGS) -c delaunay.cc 

rand_poly_gen_driver: rand_poly_gen_driver.o random_polygon_generator.o random_polygon_generator.h helper.h logger.h polygon_new.h
	$(CXX) $(CXXFLAGS) -o rand_poly_gen_driver rand_poly_gen_driver.o $(OBJ)

rand_poly_gen_driver.o: rand_poly_gen_driver.cc 
	$(CXX) $(CXXFLAGS) -c rand_poly_gen_driver.cc

clean:
	rm -f *.o $(EXE) 

