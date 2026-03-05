.PHONY: deps configure build run clean

CMAKE ?= cmake
BUILD_DIR ?= build
JOBS ?=

deps:
	@echo "Debian/Ubuntu dependencies:"
	@echo "  sudo apt install build-essential cmake libx11-dev libegl1-mesa-dev libgles1-mesa-dev libpng-dev"

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	$(CMAKE) --build $(BUILD_DIR) $(if $(JOBS),--parallel $(JOBS),)

run: build
	./$(BUILD_DIR)/pewiku

clean:
	rm -rf $(BUILD_DIR)
