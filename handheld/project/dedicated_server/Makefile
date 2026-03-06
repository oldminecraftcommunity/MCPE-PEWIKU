ROOT_DIR := ../../..

.PHONY: build run clean

build:
	$(MAKE) -C $(ROOT_DIR) build-server

run: build
	$(MAKE) -C $(ROOT_DIR) run-server

clean:
	$(MAKE) -C $(ROOT_DIR) clean
