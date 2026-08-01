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
