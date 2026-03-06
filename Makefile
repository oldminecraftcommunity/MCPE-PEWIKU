.PHONY: build-all deps configure build-linux build-server clean

BUILD_DIR ?= build
CMAKE ?= cmake
JOBS ?= $(shell nproc 2>/dev/null || echo 12)

build-all: build-linux build-server

deps:
	@echo "Debian/Ubuntu dependencies:"
	@echo "  sudo apt install build-essential cmake libx11-dev libegl1-mesa-dev libgles1-mesa-dev libpng-dev"

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build-linux: configure
	@echo "Jobs: " $(JOBS)
	$(CMAKE) --build $(BUILD_DIR) --target pewiku_linux -j $(JOBS)

build-server: configure
	@echo "Jobs:" $(JOBS)
	$(CMAKE) --build $(BUILD_DIR) --target pewiku_server -j $(JOBS)

run-linux: build-linux
	./$(BUILD_DIR)/handheld/project/linux/pewiku_linux

run-server: build-server
	./$(BUILD_DIR)/handheld/project/dedicated_server/pewiku_server

clean:
	rm -rf $(BUILD_DIR)
