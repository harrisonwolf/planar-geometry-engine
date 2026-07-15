CXX := g++
CXX_VERSION_LINE := $(shell $(CXX) --version 2>/dev/null | sed -n '1p')
BASE_CXXFLAGS := -Wall -Wextra -std=c++17
DEPFLAGS := -MMD -MP
RELEASE_CXXFLAGS := $(BASE_CXXFLAGS) -O2
DEBUG_CXXFLAGS := $(BASE_CXXFLAGS) -g -O0 -DDEBUG
GIT_COMMIT := $(shell git rev-parse HEAD 2>/dev/null || printf unknown)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null || printf unknown)
BUILD_TIME_UTC ?= $(shell date -u +"%Y-%m-%dT%H:%M:%SZ")
GIT_DIRTY := $(shell if [ -n "$$(git status --porcelain 2>/dev/null)" ]; then printf 1; else printf 0; fi)
BUILD_INFO_CPPFLAGS := \
	-DGEOM_BUILD_COMMIT=\"$(GIT_COMMIT)\" \
	-DGEOM_BUILD_BRANCH=\"$(GIT_BRANCH)\" \
	-DGEOM_BUILD_TIME_UTC=\"$(BUILD_TIME_UTC)\" \
	-DGEOM_BUILD_DIRTY=$(GIT_DIRTY) \
	'-DGEOM_BUILD_COMPILER_COMMAND="$(CXX)"' \
	'-DGEOM_BUILD_COMPILER_VERSION="$(CXX_VERSION_LINE)"'

COMMON_EXES := main rand_poly_gen_driver driver tester
DEV_ONLY_EXES := delaunay_driver portfolio_export_driver benchmark_driver terrain_driver
PACKAGING_EXES := interview_demo
DEV_EXES := $(COMMON_EXES) $(DEV_ONLY_EXES) $(PACKAGING_EXES)
RELEASE_EXES := $(COMMON_EXES) $(PACKAGING_EXES)

SRC_DIR := src
INCLUDE_DIR := include
INTERVIEW_DIR := dist/interview
INTERVIEW_BIN_DIR := $(INTERVIEW_DIR)/bin
INTERVIEW_EXAMPLES_DIR := $(INTERVIEW_DIR)/examples
INTERVIEW_BRIDGE_DIR := $(INTERVIEW_DIR)/tools/desmos-bridge
INTERVIEW_APP_NAME := planar-geometry-demo
APP_RELEASE_DIR := dist/release
APP_RELEASE_BIN_DIR := $(APP_RELEASE_DIR)/bin
APP_RELEASE_BRIDGE_DIR := $(APP_RELEASE_DIR)/tools/desmos-bridge
APP_RELEASE_NAME := planar-geometry

# Per-executable entry sources (files containing main/test harness code).
EXE_main_SRCS := $(SRC_DIR)/main.cc
EXE_interview_demo_SRCS := $(SRC_DIR)/interview_main.cc
EXE_rand_poly_gen_driver_SRCS := $(SRC_DIR)/rand_poly_gen_driver.cc
EXE_driver_SRCS := $(SRC_DIR)/driver.cc
EXE_delaunay_driver_SRCS := $(SRC_DIR)/delaunay_driver.cc
EXE_portfolio_export_driver_SRCS := $(SRC_DIR)/portfolio_export_driver.cc
EXE_benchmark_driver_SRCS := $(SRC_DIR)/benchmark_driver.cc
EXE_terrain_driver_SRCS := $(SRC_DIR)/terrain_driver.cc $(SRC_DIR)/terrain.cc
EXE_tester_SRCS := \
	testing/tester.cc \
	testing/tdd_suite.cc \
	testing/test_assertions.cc \
	testing/suites/point_is_between_suite.cc \
	testing/suites/point_strict_is_between_suite.cc \
	testing/suites/helper_collides_suite.cc \
	testing/suites/helper_strict_collides_suite.cc \
	testing/suites/helper_is_inside_suite.cc \
	testing/suites/triangle_geometry_suite.cc \
	testing/suites/polygon_geometry_suite.cc \
	testing/suites/random_polygon_generator_suite.cc \
	testing/suites/ear_clipping_triangulation_suite.cc \
	testing/suites/delaunay_predicates_suite.cc \
	testing/suites/delaunay_triangulation_suite.cc \
	testing/suites/voronoi_suite.cc \
	testing/suites/geometry_artifact_export_suite.cc \
	testing/suites/terrain_suite.cc \
	$(SRC_DIR)/terrain.cc

# Shared implementation sources linked into each executable.
COMMON_SRCS := \
	$(SRC_DIR)/die.cc \
	$(SRC_DIR)/point.cc \
	$(SRC_DIR)/triangle.cc \
	$(SRC_DIR)/polygon.cc \
	$(SRC_DIR)/line.cc \
	$(SRC_DIR)/helper.cc \
	$(SRC_DIR)/choice.cc \
	$(SRC_DIR)/ear_clipping_triangulation.cc \
	$(SRC_DIR)/random_polygon_generator.cc \
	$(SRC_DIR)/delaunay.cc \
	$(SRC_DIR)/voronoi.cc \
	$(SRC_DIR)/polygon_app_support.cc

.PHONY: all development development-normal development-debug release release-bundle release-smoke \
	interview-release interview-smoke clean clean-development clean-development-normal \
	clean-development-debug clean-release clean-app-release clean-interview test-suite help \
	generate-build-info benchmark-smoke benchmark-standard benchmark-headline benchmark-research \
	benchmark-validate benchmark-tools-test FORCE

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
$(1)_EXES := $$($(5))
$(1)_TARGETS := $$(addprefix $$($(1)_DIR)/,$$($(1)_EXES))
$(1)_ENTRY_SRCS := $$(sort $$(foreach exe,$$($(1)_EXES),$$(EXE_$$(exe)_SRCS)))
$(1)_SRCS := $$(COMMON_SRCS) $$($(1)_ENTRY_SRCS)
$(1)_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$($(1)_SRCS))
$(1)_DEPS := $$($(1)_OBJS:.o=.d)
$(1)_COMMON_OBJS := $$(patsubst %.cc,$$($(1)_DIR)/%.o,$$(COMMON_SRCS))

$$($(1)_DIR):
	mkdir -p $$@

$$($(1)_DIR)/%.o: %.cc | $$($(1)_DIR)
	mkdir -p $$(dir $$@)
	$$(CXX) $$($(3)) $(BUILD_INFO_CPPFLAGS) '-DGEOM_BUILD_COMPILER_FLAGS="$$($(3))"' -DGEOM_BUILD_PROFILE=\"$(4)\" -I$(INCLUDE_DIR) $$(DEPFLAGS) -c $$< -o $$@

$$(foreach exe,$$($(1)_EXES),$$(eval $$(call EXE_RULE,$(1),$$(exe),$(3))))

-include $$($(1)_DEPS)
endef

$(eval $(call BUILD_CONFIG,DEV_NORMAL,build/development/normal,RELEASE_CXXFLAGS,development-normal,DEV_EXES))
$(eval $(call BUILD_CONFIG,DEV_DEBUG,build/development/debug,DEBUG_CXXFLAGS,development-debug,DEV_EXES))
$(eval $(call BUILD_CONFIG,RELEASE,build/release,RELEASE_CXXFLAGS,release,RELEASE_EXES))
FORCE:

build/development/normal/src/benchmark_driver.o: FORCE


development-normal: $(DEV_NORMAL_TARGETS)
development-debug: $(DEV_DEBUG_TARGETS)
release: $(RELEASE_TARGETS)

test-suite: build/development/normal/tester
	./build/development/normal/tester
	$(MAKE) build/development/normal/terrain_driver
	bash testing/terrain_driver_smoke.sh build/development/normal/terrain_driver

BENCHMARK_BINARY := build/development/normal/benchmark_driver
BENCHMARK_OUTPUT_ROOT ?= benchmarks/runs
BENCHMARK_RUN_ID_ARG := $(if $(RUN_ID),--run-id=$(RUN_ID),)
BENCHMARK_ALLOW_DIRTY_ARG := $(if $(ALLOW_DIRTY),--allow-dirty,)
BENCHMARK_BUILD_ARGS := --compiler=$(CXX) --compiler-flags='$(RELEASE_CXXFLAGS)' --expected-build-profile=development-normal

benchmark-smoke: $(BENCHMARK_BINARY)
	python3 benchmarks/run_benchmarks.py --profile=smoke --binary=$(BENCHMARK_BINARY) --output-root=$(BENCHMARK_OUTPUT_ROOT) $(BENCHMARK_BUILD_ARGS) $(BENCHMARK_RUN_ID_ARG) $(BENCHMARK_ALLOW_DIRTY_ARG)

benchmark-standard: $(BENCHMARK_BINARY)
	python3 benchmarks/run_benchmarks.py --profile=standard --binary=$(BENCHMARK_BINARY) --output-root=$(BENCHMARK_OUTPUT_ROOT) $(BENCHMARK_BUILD_ARGS) $(BENCHMARK_RUN_ID_ARG) $(BENCHMARK_ALLOW_DIRTY_ARG)

benchmark-headline: $(BENCHMARK_BINARY)
	python3 benchmarks/run_benchmarks.py --profile=headline --binary=$(BENCHMARK_BINARY) --output-root=$(BENCHMARK_OUTPUT_ROOT) $(BENCHMARK_BUILD_ARGS) $(BENCHMARK_RUN_ID_ARG) $(BENCHMARK_ALLOW_DIRTY_ARG)

benchmark-research: $(BENCHMARK_BINARY)
	python3 benchmarks/run_benchmarks.py --profile=research --binary=$(BENCHMARK_BINARY) --output-root=$(BENCHMARK_OUTPUT_ROOT) $(BENCHMARK_BUILD_ARGS) $(BENCHMARK_RUN_ID_ARG) $(BENCHMARK_ALLOW_DIRTY_ARG)


benchmark-validate:
	@test -n "$(BUNDLE)" || (echo "Usage: make benchmark-validate BUNDLE=benchmarks/runs/<run-id>" >&2; exit 2)
	python3 benchmarks/validate_bundle.py $(BUNDLE)
benchmark-tools-test: $(BENCHMARK_BINARY)
	python3 -m unittest discover -s benchmarks -p 'test_*.py'



generate-build-info:
	GIT_COMMIT="$(GIT_COMMIT)" GIT_BRANCH="$(GIT_BRANCH)" BUILD_TIME_UTC="$(BUILD_TIME_UTC)" GIT_DIRTY="$(GIT_DIRTY)" ./scripts/generate_build_info.sh tools/desmos-bridge/build-info.js

interview-release: test-suite release generate-build-info
	rm -rf $(INTERVIEW_DIR)
	mkdir -p $(INTERVIEW_BIN_DIR)
	mkdir -p $(INTERVIEW_EXAMPLES_DIR)
	mkdir -p $(INTERVIEW_BRIDGE_DIR)
	cp build/release/interview_demo $(INTERVIEW_BIN_DIR)/$(INTERVIEW_APP_NAME)
	cp packaging/interview/run-demo.sh $(INTERVIEW_DIR)/run-demo.sh
	cp packaging/interview/QUICKSTART.md $(INTERVIEW_DIR)/QUICKSTART.md
	cp examples/interview-demo-polygon.txt $(INTERVIEW_EXAMPLES_DIR)/interview-demo-polygon.txt
	cp tools/desmos-bridge/index.html $(INTERVIEW_BRIDGE_DIR)/index.html
	cp tools/desmos-bridge/build-info.js $(INTERVIEW_BRIDGE_DIR)/build-info.js
	chmod +x $(INTERVIEW_BIN_DIR)/$(INTERVIEW_APP_NAME)
	chmod +x $(INTERVIEW_DIR)/run-demo.sh
	cd $(INTERVIEW_DIR) && ./bin/$(INTERVIEW_APP_NAME) --run-sample-demo --no-browser-launch >/dev/null

release-bundle: test-suite release generate-build-info
	rm -rf $(APP_RELEASE_DIR)
	mkdir -p $(APP_RELEASE_BIN_DIR)
	mkdir -p $(APP_RELEASE_BRIDGE_DIR)
	cp build/release/main $(APP_RELEASE_BIN_DIR)/$(APP_RELEASE_NAME)
	cp packaging/release/run.sh $(APP_RELEASE_DIR)/run.sh
	cp packaging/release/QUICKSTART.md $(APP_RELEASE_DIR)/QUICKSTART.md
	cp tools/desmos-bridge/index.html $(APP_RELEASE_BRIDGE_DIR)/index.html
	cp tools/desmos-bridge/build-info.js $(APP_RELEASE_BRIDGE_DIR)/build-info.js
	chmod +x $(APP_RELEASE_BIN_DIR)/$(APP_RELEASE_NAME)
	chmod +x $(APP_RELEASE_DIR)/run.sh

release-smoke: release-bundle
	bash ./scripts/release_smoke.sh $(APP_RELEASE_DIR)

interview-smoke: interview-release
	bash ./scripts/interview_smoke.sh $(INTERVIEW_DIR)

help:
	@echo "Common targets:"
	@echo "  make development-normal"
	@echo "  make development-debug"
	@echo "  make release"
	@echo "  make release-bundle"
	@echo "  make release-smoke"
	@echo "  make interview-release"
	@echo "  make interview-smoke"
	@echo "  make test-suite"
	@echo "  make benchmark-smoke"
	@echo "  make benchmark-standard"
	@echo "  make benchmark-headline"
	@echo "  make benchmark-research"
	@echo "  make benchmark-validate BUNDLE=benchmarks/runs/<run-id>"
	@echo "  make benchmark-tools-test"
	@echo "  make build/development/normal/terrain_driver"
	@echo ""
	@echo "See README.md for developer and interview packaging workflows."
	@echo ""
	@echo "To add a new executable:"
	@echo "  1) Add its name to EXES"
	@echo "  2) Define EXE_<name>_SRCS with its entry .cc files"
	@echo "To add shared code: add .cc files to COMMON_SRCS"

clean: clean-development clean-release clean-app-release clean-interview

clean-development: clean-development-normal clean-development-debug

clean-development-normal:
	rm -rf build/development/normal

clean-development-debug:
	rm -rf build/development/debug

clean-release:
	rm -rf build/release

clean-app-release:
	rm -rf $(APP_RELEASE_DIR)

clean-interview:
	rm -rf $(INTERVIEW_DIR)
