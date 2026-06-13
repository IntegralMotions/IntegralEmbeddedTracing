#pragma once

#include <functional>

#include "Trace.h"

/**
 * @brief Represents a traceable array with associated metadata and update handlers.
 */
struct ArrayTrace : public Trace {
    uint32_t length;                            ///< Length of the array.
    std::function<void()> checkForUpdate;       ///< Callback to check for updates to the variable.
    std::function<void(uint32_t index)> update; ///< Callback to update the variable.
};
