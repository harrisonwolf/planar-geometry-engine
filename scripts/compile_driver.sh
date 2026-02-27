mkdir -p bin
g++ -I. -o bin/driver \
  drivers/driver.cc \
  die.cc point.cc triangle.cc polygon_new.cc line.cc helper.cc choice.cc \
  ear_clipping_triangulation.cc random_polygon_generator.cc delaunay.cc
