SHELL := /bin/bash

BUILD_DIR ?= build
CMAKE     ?= cmake
JOBS      ?= $(shell nproc 2>/dev/null || echo 1)
CONFIG    ?= Debug
BIN_NAME  ?= iris
BIN       := $(BIN_NAME)

.PHONY: all configure build run install clean distclean check-deps deps help

all: build

# ── dependency checks ────────────────────────────────────────────────────────

check-deps:
	@echo "Checking build dependencies..."
	@MISSING=""; \
	command -v cmake  >/dev/null 2>&1 || MISSING="$$MISSING cmake"; \
	command -v gcc    >/dev/null 2>&1 || MISSING="$$MISSING gcc"; \
	command -v g++    >/dev/null 2>&1 || MISSING="$$MISSING g++"; \
	command -v make   >/dev/null 2>&1 || MISSING="$$MISSING make"; \
	command -v pkg-config >/dev/null 2>&1 || MISSING="$$MISSING pkg-config"; \
	pkg-config --exists libcurl 2>/dev/null   || MISSING="$$MISSING libcurl-dev"; \
	pkg-config --exists openssl 2>/dev/null   || MISSING="$$MISSING libssl-dev"; \
	if [ -n "$$MISSING" ]; then \
		echo "Missing:$$MISSING"; \
		echo "Installing..."; \
		if [ ! -f /etc/os-release ]; then \
			echo "Cannot detect OS. Install manually:$$MISSING"; exit 1; \
		fi; \
		. /etc/os-release; \
		case "$$ID" in \
			ubuntu|debian) \
				sudo apt-get update -qq && \
				sudo apt-get install -y build-essential cmake libcurl4-openssl-dev libssl-dev pkg-config ;; \
			fedora|rhel|centos) \
				sudo dnf install -y gcc gcc-c++ make cmake libcurl-devel openssl-devel pkgconfig ;; \
			arch|manjaro) \
				sudo pacman -S --needed --noconfirm base-devel cmake curl openssl pkg-config ;; \
			*) \
				echo "Unsupported OS: $$ID. Install manually:$$MISSING"; exit 1 ;; \
		esac; \
	else \
		echo "All dependencies present."; \
	fi

deps: check-deps
	@mkdir -p vendor
	@if [ ! -f vendor/libgit2-v1.8.5.zip ]; then \
		wget -O vendor/libgit2-v1.8.5.zip https://github.com/libgit2/libgit2/archive/refs/tags/v1.8.5.zip; \
	fi
	@if [ ! -d vendor/libgit2-1.8.5 ]; then \
		unzip -o vendor/libgit2-v1.8.5.zip -d vendor/; \
	fi

# ── build targets ────────────────────────────────────────────────────────────

configure: deps
	@echo "Configuring (BUILD_DIR=$(BUILD_DIR), CONFIG=$(CONFIG))..."
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) $(CMAKE_OPTIONS)

build: configure
	@echo "Building ($(JOBS) jobs) in $(BUILD_DIR)..."
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(JOBS)

run: build
	@echo "Running $(BIN)"
	@./$(BUILD_DIR)/$(BIN)

# ── install ──────────────────────────────────────────────────────────────────

install: build
	@echo "Installing iris to /usr/local/bin..."
	@sudo install -m 755 $(BUILD_DIR)/$(BIN) /usr/local/bin/$(BIN)
	@SHELL_RC=""; \
	case "$$SHELL" in \
		*/zsh)  SHELL_RC="$$HOME/.zshrc"  ;; \
		*/bash) SHELL_RC="$$HOME/.bashrc" ;; \
		*)      SHELL_RC="$$HOME/.bashrc" ;; \
	esac; \
	if ! grep -q "alias iris=" "$$SHELL_RC" 2>/dev/null; then \
		echo "" >> "$$SHELL_RC"; \
		echo "alias iris='/usr/local/bin/iris'" >> "$$SHELL_RC"; \
		echo "Alias added to $$SHELL_RC"; \
	else \
		echo "Alias already in $$SHELL_RC"; \
	fi; \
	echo "Done. Run: source $$SHELL_RC"

# ── misc ─────────────────────────────────────────────────────────────────────

ctest: build
	@cd $(BUILD_DIR) && ctest --output-on-failure || true

clean:
	@if [ -d $(BUILD_DIR) ]; then $(CMAKE) --build $(BUILD_DIR) --target clean || true; fi
	@if [ -d $(BUILD_DIR) ]; then rm -f $(BUILD_DIR)/CMakeCache.txt; fi
	@$(MAKE) configure

distclean:
	@echo "Removing build directory $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

help:
	@echo "Targets:"
	@echo "  all          (default) configure + build"
	@echo "  check-deps   Detect and install missing system deps"
	@echo "  configure    Run cmake configure"
	@echo "  build        Build the project"
	@echo "  run          Build and run iris"
	@echo "  install      Install to /usr/local/bin + add shell alias"
	@echo "  ctest        Run tests"
	@echo "  clean        Clean build artifacts"
	@echo "  distclean    Remove build directory entirely"
	@echo "Vars: BUILD_DIR CONFIG CMAKE JOBS CMAKE_OPTIONS"
