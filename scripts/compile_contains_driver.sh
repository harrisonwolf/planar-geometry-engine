mkdir -p build/development/normal
g++ -Iinclude -o build/development/normal/contains_driver src/contains_driver.cc src/die.cc src/point.cc src/line.cc src/triangle.cc
