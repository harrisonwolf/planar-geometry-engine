mkdir -p bin
g++ -I. -o bin/ear_clipper drivers/ear_clipper.cc ear_clipping_triangulation.cc die.cc point.cc line.cc triangle.cc polygon_new.cc helper.cc
