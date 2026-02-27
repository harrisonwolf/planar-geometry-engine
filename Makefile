CXX := g++
BASE_CXXFLAGS := -Wall -Wextra -std=c++17
RELEASE_CXXFLAGS := $(BASE_CXXFLAGS) -O2
DEBUG_CXXFLAGS := $(BASE_CXXFLAGS) -g -O0 -DDEBUG

EXES := main rand_poly_gen_driver #driver

COMMON_SRCS := \
	die.cc \
	point.cc \
	triangle.cc \
	polygon_new.cc \
	line.cc \
	helper.cc \
	choice.cc \
	ear_clipping_triangulation.cc \
	random_polygon_generator.cc \
	delaunay.cc

.PHONY: all release debug clean clean-release clean-debug

all: release debug

release: build/release/main build/release/rand_poly_gen_driver build/release/driver

debug: build/debug/main build/debug/rand_poly_gen_driver build/debug/driver

# Build template for each configuration.
define BUILD_CONFIG
$(1)_DIR := build/$(1)
$(1)_COMMON_OBJS := $$(COMMON_SRCS:%.cc=$$($(1)_DIR)/%.o)
$(1)_MAIN_OBJS := $$($(1)_DIR)/main.o $$($(1)_COMMON_OBJS)
$(1)_RAND_POLY_OBJS := $$($(1)_DIR)/rand_poly_gen_driver.o $$($(1)_COMMON_OBJS)
$(1)_DRIVER_OBJS := $$($(1)_DIR)/driver.o $$($(1)_COMMON_OBJS)

$$($(1)_DIR):
	mkdir -p $$@

$$($(1)_DIR)/main: $$($(1)_MAIN_OBJS)
	$$(CXX) $$($(2)) -o $$@ $$($(1)_MAIN_OBJS)

$$($(1)_DIR)/rand_poly_gen_driver: $$($(1)_RAND_POLY_OBJS)
	$$(CXX) $$($(2)) -o $$@ $$($(1)_RAND_POLY_OBJS)

$$($(1)_DIR)/driver: $$($(1)_DRIVER_OBJS)
	$$(CXX) $$($(2)) -o $$@ $$($(1)_DRIVER_OBJS)

$$($(1)_DIR)/main.o: main.cc logger.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/rand_poly_gen_driver.o: rand_poly_gen_driver.cc | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/driver.o: driver.cc | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/die.o: die.cc die.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/point.o: point.cc point.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/triangle.o: triangle.cc triangle.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/polygon_new.o: polygon_new.cc polygon_new.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/line.o: line.cc line.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/helper.o: helper.cc helper.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/choice.o: choice.cc choice.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/ear_clipping_triangulation.o: ear_clipping_triangulation.cc ear_clipping_triangulation.h logger.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/random_polygon_generator.o: random_polygon_generator.cc random_polygon_generator.h polygon_new.h logger.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@

$$($(1)_DIR)/delaunay.o: delaunay.cc delaunay.h triangle.h | $$($(1)_DIR)
	$$(CXX) $$($(2)) -c $$< -o $$@
endef

$(eval $(call BUILD_CONFIG,release,RELEASE_CXXFLAGS))
$(eval $(call BUILD_CONFIG,debug,DEBUG_CXXFLAGS))

clean: clean-release clean-debug

clean-release:
	rm -rf build/release

clean-debug:
	rm -rf build/debug
