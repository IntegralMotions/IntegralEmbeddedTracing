#!/usr/bin/env python3
"""Smoke-test the virtual trace device using only the Python standard library."""

import binascii
import os
import select
import signal
import subprocess
import sys
import termios
import time
import tty


def encode(payload):
    crc = binascii.crc_hqx(payload, 0xFFFF)
    data = payload + crc.to_bytes(2, "little")
    encoded = bytearray([0])
    code_index = 0
    code = 1
    for byte in data:
        if byte == 0:
            encoded[code_index] = code
            code_index = len(encoded)
            encoded.append(0)
            code = 1
        else:
            encoded.append(byte)
            code += 1
            if code == 0xFF:
                encoded[code_index] = code
                code_index = len(encoded)
                encoded.append(0)
                code = 1
    encoded[code_index] = code
    return encoded + b"\0"


def decode(frame):
    result = bytearray()
    index = 0
    while index < len(frame):
        code = frame[index]
        if code == 0 or index + code > len(frame):
            raise ValueError("Invalid COBS frame")
        index += 1
        result.extend(frame[index:index + code - 1])
        index += code - 1
        if code != 0xFF and index < len(frame):
            result.append(0)
    if len(result) < 3 or binascii.crc_hqx(result[:-2], 0xFFFF) != int.from_bytes(result[-2:], "little"):
        raise ValueError("Invalid CRC")
    return result[:-2]


def receive(fd, pending, timeout=3):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if b"\0" in pending:
            frame, _, rest = pending.partition(b"\0")
            pending[:] = rest
            return decode(frame)
        ready, _, _ = select.select([fd], [], [], max(0, deadline - time.monotonic()))
        if ready:
            pending.extend(os.read(fd, 4096))
    raise TimeoutError("Timed out waiting for a trace frame")


def main():
    executable = sys.argv[1] if len(sys.argv) > 1 else "build/TracingSerialDevice"
    process = subprocess.Popen([executable], stdout=subprocess.PIPE, text=True)
    fd = None
    try:
        line = process.stdout.readline().strip()
        if not line.startswith("Serial port: "):
            raise RuntimeError(f"Device did not provide a serial port: {line}")
        fd = os.open(line.removeprefix("Serial port: "), os.O_RDWR | os.O_NOCTTY)
        tty.setraw(fd, termios.TCSANOW)
        pending = bytearray()

        os.write(fd, encode(bytes([1])))
        config = receive(fd, pending)
        assert config[:2] == bytes([2, 4]) and all(
            name in config for name in (b"current_u", b"current_v", b"current_w", b"counter")
        ), config

        os.write(fd, encode(bytes([5, 4, 0, 1, 2, 3])))
        start = receive(fd, pending)
        assert start == bytes([6, 4, 0, 0, 1, 1, 2, 2, 3, 3, 0]), start
        update = receive(fd, pending)
        assert update[0] == 7 and update[1] > 0, update

        os.write(fd, encode(bytes([8])))
        print("OK: configuration, start response, and trace data received over", line.removeprefix("Serial port: "))
    finally:
        if fd is not None:
            os.close(fd)
        process.send_signal(signal.SIGTERM)
        process.wait(timeout=3)


if __name__ == "__main__":
    main()
