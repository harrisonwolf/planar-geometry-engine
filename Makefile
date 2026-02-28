CXX := g++
BASE_CXXFLAGS := -Wall -Wextra -std=c++17
DEPFLAGS := -MMD -MP
RELEASE_CXXFLAGS := $(BASE_CXXFLAGS) -O2
DEBUG_CXXFLAGS := $(BASE_CXXFLAGS) -g -O0 -DDEBUG

# Add/remove executable names in one place.
EXES := main rand_poly_gen_driver driver tester

# Per-executable entry sources (files containing main/test harness code).
EXE_main_SRCS := main.cc
EXE_rand_poly_gen_driver_SRCS := rand_poly_gen_driver.cc
EXE_driver_SRCS := driver.cc
EXE_tester_SRCS := testing/tester.cc testing/tdd_suite.cc

# Shared implementation sources linked into each executable.
COMMON_SRCS := \
	die.cc \
	point.cc \
	triangle.cc \
	polygon.cc \
	line.cc \
	helper.cc \
	choice.cc \
	ear_clipping_triangulation.cc \
	random_polygon_generator.cc \
	delaunay.cc

.PHONY: all development development-normal development-debug release clean \
	clean-development clean-development-normal clean-development-debug clean-release \
	test-suite help

all: development release

development: development-normal development-debug

# Generates per-executable object lists and link rules for one build config.
define EXE_RULE
$(1)_$(2)_ENTRY_SRCS := $$(EXE_$(2)_SRCS)
$(1)_$(2)_ENTRY_OBJS := $$(addprefix $$($(1)_DIR)/,$$(patsubst %.cc,%.o,$$($(1)_$(2)_ENTRY_SRCS)))
$(1)_$(2)_OBJS := $$($(1)_$(2)_ENTRY_OBJS) $$($(1)_COMMON_OBJS)

$$($(1)_DIR)/$(2): $$($(1)_$(2)_OBJS)
	$$(CXX) $$($(3)) -o $$@ $$^
endef

# Build template for each configuration.
define BUILD_CONFIG
$(1)_DIR := $(2)
$(1)_TARGETS := $$(addprefix $$($(1)_DIR)/,$$(EXES))
$(1)_ENTRY_SRCS := $$(sort $$(foreach exe,$$(EXES),$$(EXE_$$(exe)_SRCS)))
$(1)_SRCS := $$(COMMON_SRCS) $$($(1)_ENTRY_SRCS)
$(1)_OBJS := $$($(1)_SRCS:%.cc=$$($(1)_DIR)/%.o)
$(1)_DEPS := $$($(1)_OBJS:.o=.d)
$(1)_COMMON_OBJS := $$(COMMON_SRCS:%.cc=$$($(1)_DIR)/%.o)

$$($(1)_DIR):
	mkdir -p $$@

$$($(1)_DIR)/%.o: %.cc | $$($(1)_DIR)
	mkdir -p $$(dir $$@)
	$$(CXX) $$($(3)) $$(DEPFLAGS) -c $$< -o $$@

$$(foreach exe,$$(EXES),$$(eval $$(call EXE_RULE,$(1),$$(exe),$(3))))

-include $$($(1)_DEPS)
endef

$(eval $(call BUILD_CONFIG,DEV_NORMAL,build/development/normal,RELEASE_CXXFLAGS))
$(eval $(call BUILD_CONFIG,DEV_DEBUG,build/development/debug,DEBUG_CXXFLAGS))
$(eval $(call BUILD_CONFIG,RELEASE,build/release,RELEASE_CXXFLAGS))

development-normal: $(DEV_NORMAL_TARGETS)
development-debug: $(DEV_DEBUG_TARGETS)
release: $(RELEASE_TARGETS)

test-suite: build/development/normal/tester
	./build/development/normal/tester

help:
	@echo "Common targets:"
	@echo "  make development-normal"
	@echo "  make development-debug"
	@echo "  make release"
	@echo "  make test-suite"
	@echo ""
	@echo "To add a new executable:"
	@echo "  1) Add its name to EXES"
	@echo "  2) Define EXE_<name>_SRCS with its entry .cc files"
	@echo "To add shared code: add .cc files to COMMON_SRCS"

clean: clean-development clean-release

clean-development: clean-development-normal clean-development-debug

clean-development-normal:
	rm -rf build/development/normal

clean-development-debug:
	rm -rf build/development/debug

clean-release:
	rm -rf build/release
