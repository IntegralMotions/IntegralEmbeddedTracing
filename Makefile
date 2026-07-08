BUILD_DIR  := build
GENERATOR  := Unix Makefiles
BUILD_TYPE ?= Debug  # default if not overridden

.PHONY: build test clean configure debug release

configure:
	mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR) -G "$(GENERATOR)" \
	      -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	      -DCMAKE_CXX_STANDARD=17 \

build: configure
	cmake --build $(BUILD_DIR) --parallel

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

debug:
	$(MAKE) BUILD_TYPE=Debug build

release:
	$(MAKE) BUILD_TYPE=Release build

clean:
	rm -rf $(BUILD_DIR)
