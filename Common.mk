# Shared Integral Motions C++ build tooling

.PHONY: configure build test test-all debug release clean format tidy update

configure:
	cmake -S . -B "$(BUILD_DIR)" -G "$(GENERATOR)" \
	-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-D$(DEPENDENCY_TESTS_OPTION)="$(DEPENDENCY_TESTS)"

build: configure
	cmake --build "$(BUILD_DIR)" --parallel

test: build
	@if [ -n "$(TEST_LABEL)" ]; then \
	ctest --test-dir "$(BUILD_DIR)" \
	--output-on-failure \
	-L "$(TEST_LABEL)"; \
	else \
	ctest --test-dir "$(BUILD_DIR)" \
	--output-on-failure; \
	fi

test-all: DEPENDENCY_TESTS := ON
test-all: build
	ctest --test-dir "$(BUILD_DIR)" --output-on-failure

debug:
	$(MAKE) BUILD_TYPE=Debug build

release:
	$(MAKE) BUILD_TYPE=Release build

clean:
	cmake -E remove_directory "$(BUILD_DIR)"

format:
	@command -v clang-format >/dev/null 2>&1 || { \
	echo "Error: clang-format is not installed."; \
	exit 1; \
	}
	find . \
	-type f \
	\( \
	-name '*.c' \
	-o -name '*.cc' \
	-o -name '*.cpp' \
	-o -name '*.cxx' \
	-o -name '*.h' \
	-o -name '*.hh' \
	-o -name '*.hpp' \
	-o -name '*.hxx' \
	\) \
	-not -path './$(BUILD_DIR)/*' \
	-exec clang-format -i {} +

tidy: configure
	@command -v run-clang-tidy >/dev/null 2>&1 || { \
	echo "Error: run-clang-tidy is not installed."; \
	exit 1; \
	}
	run-clang-tidy -p "$(BUILD_DIR)"

update:
	@set -eu; \
	temp_dir=$$(mktemp -d); \
	trap 'rm -rf "$$temp_dir"' EXIT; \
	echo "Downloading tooling from $(TOOLING_REPOSITORY)@$(TOOLING_VERSION)..."; \
	curl -fsSL \
	"$(TOOLING_URL)/Common.mk" \
	-o "$$temp_dir/Common.mk"; \
	curl -fsSL \
	"$(TOOLING_URL)/.clang-format" \
	-o "$$temp_dir/.clang-format"; \
	curl -fsSL \
	"$(TOOLING_URL)/.clang-tidy" \
	-o "$$temp_dir/.clang-tidy"; \
	install -m 644 "$$temp_dir/Common.mk" Common.mk; \
	install -m 644 "$$temp_dir/.clang-format" .clang-format; \
	install -m 644 "$$temp_dir/.clang-tidy" .clang-tidy; \
	echo "Tooling updated successfully."; \
	echo "Review the changes with: git diff"
