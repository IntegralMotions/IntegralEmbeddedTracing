#pragma once

#include "Trace.h"

namespace IntegralMotions::Tracing {
    /**
     * @brief Represents a traceable variable with associated metadata and update handlers.
     */
    struct VariableTrace : public Trace {};
} // namespace IntegralMotions::Tracing