#pragma once

#include "IntegralEmbeddedTracing/TraceDataHeader.h"
#include "IntegralEmbeddedTracing/TraceTypes.h"

/**
 * @brief Represents a traceable item with associated metadata and update handlers.
 */
struct Trace {
  public:
    const char* name;             ///< Name of the variable.
    uint16_t id;                  ///< Unique identifier for the variable.
    TraceValueType type;          ///< Type of the variable.
    uint8_t typeSize;             ///< Size of the type of the variable.
    uint8_t* value;               ///< Pointer to the variable's value.
    uint8_t* previousValue;       ///< Pointer to the variable's previous value.
    TraceDataHeader updateHeader; ///< The header that is used when sending an update
};
