BUILD_DIR := build
GENERATOR := Unix Makefiles
BUILD_TYPE ?= Debug

DEPENDENCY_TESTS ?= OFF
DEPENDENCY_TESTS_OPTION := INTEGRAL_BUILD_DEPENDENCY_TESTS

TEST_LABEL := IntegralEmbeddedTracing

TOOLING_VERSION := main
TOOLING_REPOSITORY ?= IntegralMotions/CppTooling
TOOLING_URL := https://raw.githubusercontent.com/$(TOOLING_REPOSITORY)/$(TOOLING_VERSION)

include Common.mk

.PHONY: example
example: BUILD_TYPE = Release
example:
	cmake -S . -B "$(BUILD_DIR)" -G "$(GENERATOR)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-D$(DEPENDENCY_TESTS_OPTION)="$(DEPENDENCY_TESTS)" \
		-DINTEGRAL_BUILD_EXAMPLES=ON
	cmake --build "$(BUILD_DIR)" --target TracingSerialDevice --parallel
	"$(BUILD_DIR)/TracingSerialDevice"
