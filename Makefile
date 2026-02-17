all: main

main: main.o die.o point.o triangle.o polygon_new.o line.o helper.o choice.o ear_clipping_triangulation.o
	g++ -o main main.o die.o point.o triangle.o polygon_new.o line.o helper.o choice.o ear_clipping_triangulation.o

die.o: die.cc die.h
	g++ -c die.cc

point.o: point.cc point.h
	g++ -c point.cc

triangle.o: triangle.cc triangle.h
	g++ -c triangle.cc

polygon_new.o: polygon_new.cc polygon_new.h
	g++ -c polygon_new.cc

line.o: line.cc line.h
	g++ -c line.cc

helper.o: helper.cc helper.h
	g++ -c helper.cc

choice.o: choice.cc choice.h
	g++ -c choice.cc

ear_clipping_triangulation.o: ear_clipping_triangulation.cc ear_clipping_triangulation.h
	g++ -c ear_clipping_triangulation.cc
