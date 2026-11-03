# Virtual serial trace device

This example runs the real `TraceController` against a pseudo-terminal on macOS
or Linux. It prints the serial port to open in your host application. No physical
serial hardware, driver, or `socat` is required on those platforms.

```sh
make example
```

This builds and runs the device. Alternatively, use CMake directly:

```sh
cmake -S . -B build -DINTEGRAL_BUILD_EXAMPLES=ON
cmake --build build --target TracingSerialDevice
./build/TracingSerialDevice
```

On Windows, first create a connected virtual COM-port pair using a driver such
as com0com. Build with CMake, then start the example with one port and connect
the host to the other, for example:

```powershell
cmake -S . -B build -DINTEGRAL_BUILD_EXAMPLES=ON -DINTEGRAL_BUILD_TESTS=OFF
cmake --build build --config Release --target TracingSerialDevice
./build/Release/TracingSerialDevice.exe COM30
# Connect the host to the paired COM31 port.
```

To smoke-test the virtual device independently of your host, run
`python3 examples/test_serial_device.py` after building it. An alternate path to
the executable can be supplied as the script's first argument.

To test the Python/C++ TracerLL receiver against a running device, install
TracerLL and pytest in a Python environment, start `make example` in another
terminal, and run:

```sh
cd /path/to/TracerLL
TRACER_SERIAL_TEST=1 TRACER_SERIAL_PORT=/dev/ttys016 python -m pytest -v tests/integration/test_serial_tracing.py
```

Use the port printed by the example; `TRACER_SERIAL_PORT` defaults to
`/dev/ttys016` for this manual test. Pseudo-terminal names can change between
runs, so verify the path before connecting. The test sends trace requests and a
stop event; don't run it against a real device or alongside another client.

Open the printed `/dev/pts/...` (Linux) or `/dev/ttys...` (macOS) path with your
serial code. The path exists only while the example runs. A pseudo-terminal has no
physical baud rate; configure the host for 115200, 8N1 with no flow control.
TracerLL's `UsbCommunication` can open this port on macOS/Linux using a numeric
baud rate such as `115200`. Press Ctrl-C to stop the example.

The device registers these variables in order:

| ID | Name | Type | Behavior |
| -- | ---- | ---- | -------- |
| 0 | `current_u` | float | Phase U current in amperes |
| 1 | `current_v` | float | Phase V current in amperes |
| 2 | `current_w` | float | Phase W current in amperes |
| 3 | `counter` | uint8 | Incremented each simulated interrupt; wraps at 255 |

The currents are balanced 50 Hz sine waves, 10 A peak and 120 degrees apart.
The simulator targets a 120 kHz interrupt rate (one tick every 8.33 µs). This
is not a real STM interrupt: desktop scheduling and the serial connection do not
guarantee the rate or delivery of every tick. When deadlines are missed, the
simulator skips to the current tick and advances the counter accordingly. The
counter also wraps, so it cannot reveal losses of an exact multiple of 256 ticks.
The example polls continuously and can use a full CPU core while running.

It does **not** stream unsolicited data. The host must send a `GetConfigRequest`
and then a `StartTraceRequest` selecting the desired IDs; the real controller
sends responses and subsequent updates as described in [the protocol](../docs/protocol.md).
Use `StopTraceEvent` to stop streaming. Reconnects do not automatically reset the
trace session; restart the example for a clean state.
