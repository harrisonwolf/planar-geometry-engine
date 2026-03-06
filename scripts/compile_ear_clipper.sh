mkdir -p build/development/normal
g++ -Iinclude -o build/development/normal/ear_clipper src/ear_clipper.cc src/ear_clipping_triangulation.cc src/die.cc src/point.cc src/line.cc src/triangle.cc src/polygon.cc src/helper.cc
