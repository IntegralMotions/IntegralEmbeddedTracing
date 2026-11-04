#pragma once

#include "TraceController.h"

// Destroy the singleton before its test-owned communication and logger objects.
struct TraceControllerTestGuard {
    ~TraceControllerTestGuard() { IntegralMotions::Tracing::TraceController::shutdown(); }
};
