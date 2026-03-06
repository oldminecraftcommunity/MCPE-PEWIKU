.PHONY: build-all deps configure build-client build-server run-client run-server clean

DEBUG ?= 0
BUILD_DIR := build$(if $(filter 1,$(DEBUG)),_debug,)
CMAKE ?= cmake
JOBS ?= $(shell nproc 2>/dev/null || echo 12)

build-all: build-client build-server

deps:
	@echo "Debian/Ubuntu dependencies:"
	@echo "  sudo apt install build-essential cmake libx11-dev libegl1-mesa-dev libgles1-mesa-dev libpng-dev libglew-dev libsdl2-dev libgles-dev libegl-dev pkg-config"

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DDEBUG=$(DEBUG)

build-client: configure
	@echo "Jobs: " $(JOBS)
	$(CMAKE) --build $(BUILD_DIR) --target pewiku_linux -j $(JOBS)

build-server: configure
	@echo "Jobs:" $(JOBS)
	$(CMAKE) --build $(BUILD_DIR) --target pewiku_server -j $(JOBS)

run-client: build-client
	./$(BUILD_DIR)/handheld/project/linux/pewiku_linux

run-server: build-server
	./$(BUILD_DIR)/handheld/project/dedicated_server/pewiku_server

clean:
	rm -rf $(BUILD_DIR)
