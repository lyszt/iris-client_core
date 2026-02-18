SHELL := /bin/bash

# Basic Makefile wrapper for CMake
# Usage examples:
#  make            # configure and build in Debug
#  make CONFIG=Release
#  make JOBS=4      # control parallel jobs
#  make run         # run the built binary
#  make install     # install the built binary
#  make clean       # run the cmake build clean target
#  make distclean   # remove build directory entirely

BUILD_DIR ?= build
CMAKE ?= cmake
JOBS ?= $(shell nproc 2>/dev/null || echo 1)
CONFIG ?= Debug
CMAKE_OPTIONS ?=

# Binary name (matches CMake add_executable)
BIN_NAME ?= iris
BIN := $(BUILD_DIR)/$(BIN_NAME)

.PHONY: all configure build run install clean distclean help ctest reconfigure

all: build

configure:
	@echo "Configuring (BUILD_DIR=$(BUILD_DIR), CONFIG=$(CONFIG))..."
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) $(CMAKE_OPTIONS)

build: configure
	@echo "Building ($(JOBS) jobs) in $(BUILD_DIR)..."
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(JOBS)

run: build
	@echo "Running $(BIN)"
	@$(BIN)

install: build
	@echo "Installing to /usr/local/bin by default (use DESTDIR=...)"
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(BUILD_DIR)/iris $(DESTDIR)/usr/local/bin/iris
	@$(CMAKE) --install $(BUILD_DIR) --prefix /usr/local

ctest: build
	@echo "Running tests (ctest)"
	@cd $(BUILD_DIR) && ctest --output-on-failure || true

clean:
	@if [ -d $(BUILD_DIR) ]; then \
		$(CMAKE) --build $(BUILD_DIR) --target clean || true; \
	fi

reconfigure:
	@echo "Re-configuring (removing cache)..."
	@if [ -d $(BUILD_DIR) ]; then rm -f $(BUILD_DIR)/CMakeCache.txt; fi
	@$(MAKE) configure

distclean:
	@echo "Removing build directory $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

help:
	@echo "Makefile wrapper for CMake"
	@echo "Targets:"
	@echo "  all         (default) configure + build"
	@echo "  configure   Configure cmake in $(BUILD_DIR)"
	@echo "  build       Build (runs configure first)"
	@echo "  run         Build and run the $(BIN_NAME) binary"
	@echo "  install     CMake install step (prefix=/usr/local)"
	@echo "  ctest       Run ctest (after build)"
	@echo "  clean       Run 'cmake --build build --target clean'"
	@echo "  distclean   Delete the build directory entirely"
	@echo "Environment vars: BUILD_DIR, CONFIG, CMAKE, JOBS, CMAKE_OPTIONS"
