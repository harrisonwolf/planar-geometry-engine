CXX := g++
BASE_CXXFLAGS := -Wall -Wextra -std=c++17
RELEASE_CXXFLAGS := $(BASE_CXXFLAGS) -O2
DEBUG_CXXFLAGS := $(BASE_CXXFLAGS) -g -O0 -DDEBUG
DEPFLAGS := -MMD -MP
CPPFLAGS := -I.

DRIVER_SRC_DIR := drivers
BIN_DIR := bin

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

MAIN_SRC := main.cc
RAND_POLY_SRC := $(DRIVER_SRC_DIR)/rand_poly_gen_driver.cc
DRIVER_SRC := $(DRIVER_SRC_DIR)/driver.cc

.PHONY: all development development-normal development-debug release clean \
	clean-development clean-development-normal clean-development-debug clean-release

all: development release

development: development-normal development-debug

development-normal: build/development/normal/$(BIN_DIR)/main \
	build/development/normal/$(BIN_DIR)/rand_poly_gen_driver \
	build/development/normal/$(BIN_DIR)/driver

development-debug: build/development/debug/$(BIN_DIR)/main \
	build/development/debug/$(BIN_DIR)/rand_poly_gen_driver \
	build/development/debug/$(BIN_DIR)/driver

release: build/release/$(BIN_DIR)/main \
	build/release/$(BIN_DIR)/rand_poly_gen_driver \
	build/release/$(BIN_DIR)/driver

define BUILD_CONFIG
$(1)_DIR := $(2)
$(1)_COMMON_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$(COMMON_SRCS))
$(1)_MAIN_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$(MAIN_SRC)) $$($(1)_COMMON_OBJS)
$(1)_RAND_POLY_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$(RAND_POLY_SRC)) $$($(1)_COMMON_OBJS)
$(1)_DRIVER_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$(DRIVER_SRC)) $$($(1)_COMMON_OBJS)
$(1)_DEPS := $$($(1)_MAIN_OBJS:.o=.d) $$($(1)_RAND_POLY_OBJS:.o=.d) $$($(1)_DRIVER_OBJS:.o=.d)

$$($(1)_DIR):
	mkdir -p $$@

$$($(1)_DIR)/$(BIN_DIR)/main: $$($(1)_MAIN_OBJS)
	mkdir -p $$(dir $$@)
	$$(CXX) $$(CPPFLAGS) $$($(3)) -o $$@ $$^

$$($(1)_DIR)/$(BIN_DIR)/rand_poly_gen_driver: $$($(1)_RAND_POLY_OBJS)
	mkdir -p $$(dir $$@)
	$$(CXX) $$(CPPFLAGS) $$($(3)) -o $$@ $$^

$$($(1)_DIR)/$(BIN_DIR)/driver: $$($(1)_DRIVER_OBJS)
	mkdir -p $$(dir $$@)
	$$(CXX) $$(CPPFLAGS) $$($(3)) -o $$@ $$^

$$($(1)_DIR)/%.o: %.cc | $$($(1)_DIR)
	mkdir -p $$(dir $$@)
	$$(CXX) $$(CPPFLAGS) $$($(3)) $$(DEPFLAGS) -c $$< -o $$@
endef

$(eval $(call BUILD_CONFIG,DEV_NORMAL,build/development/normal,RELEASE_CXXFLAGS))
$(eval $(call BUILD_CONFIG,DEV_DEBUG,build/development/debug,DEBUG_CXXFLAGS))
$(eval $(call BUILD_CONFIG,RELEASE,build/release,RELEASE_CXXFLAGS))

-include $(DEV_NORMAL_DEPS) $(DEV_DEBUG_DEPS) $(RELEASE_DEPS)

clean: clean-development clean-release

clean-development: clean-development-normal clean-development-debug

clean-development-normal:
	rm -rf build/development/normal

clean-development-debug:
	rm -rf build/development/debug

clean-release:
	rm -rf build/release
