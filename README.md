# Integral Embedded Tracing

Integral Embedded Tracing is a small embedded tracing library for configuring and streaming variable trace data between a host and a device.

The wire protocol is documented in [docs/protocol.md](docs/protocol.md).

For a virtual serial device to test a tracing host, see [examples](examples/README.md).

## Communication diagnostics

Pass an application-owned CoreUtilities logger when initializing the controller to
enable warnings from its internal COBS communication wrapper:

```cpp
#include <Diagnostics/BufferedLogger.h>
#include <TraceController.h>

IntegralMotions::Diagnostics::BufferedLogger<> logger;
// Attach application-owned sinks to logger.
IntegralMotions::Tracing::TraceController::init(communication, logger);
// In the application loop, call TraceController::get().loop() and logger.process().
```

The communication object and logger must outlive the controller. Only the first
`init` call takes effect. The application is responsible for processing deferred
logs and providing any needed synchronization. The original `init(communication)`
overload remains available and leaves logging disabled. With a logger, the controller
also warns about malformed or unknown messages, failed trace registration, and
truncated configuration responses. Normal polling and trace updates are not logged.

Call `TraceController::shutdown()` when tracing is no longer needed to release the
controller and its buffers. After shutdown, references returned by `get()` are
invalid; call `init()` again before using the controller. Stop using the controller
from all threads and interrupts before shutting it down. The communication object
and logger must remain alive until shutdown completes.
