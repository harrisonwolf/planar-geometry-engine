mkdir -p bin
g++ -I. -o bin/contains_driver drivers/contains_driver.cc die.cc point.cc line.cc triangle.cc
