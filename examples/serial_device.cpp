#include <IntegralCommunication/Communication.h>
#include <TraceController.h>

#include <cerrno>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif
#endif

namespace {
    volatile std::sig_atomic_t running = 1;

    void stop(int) { running = 0; }

#ifdef _WIN32
    class SerialCommunication : public Communication {
      public:
        explicit SerialCommunication(const char* port) {
            const std::string path = std::string("\\\\.\\") + port;
            _handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (_handle == INVALID_HANDLE_VALUE) return;
            DCB settings{};
            settings.DCBlength = sizeof(settings);
            if (!GetCommState(_handle, &settings)) { close(); return; }
            settings.BaudRate = 115200;
            settings.ByteSize = 8;
            settings.Parity = NOPARITY;
            settings.StopBits = ONESTOPBIT;
            settings.fOutxCtsFlow = FALSE;
            settings.fOutxDsrFlow = FALSE;
            settings.fInX = FALSE;
            settings.fOutX = FALSE;
            settings.fRtsControl = RTS_CONTROL_ENABLE;
            settings.fDtrControl = DTR_CONTROL_ENABLE;
            COMMTIMEOUTS timeouts{};
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.WriteTotalTimeoutConstant = 100;
            if (!SetCommState(_handle, &settings) || !SetCommTimeouts(_handle, &timeouts)) close();
        }
        ~SerialCommunication() override { close(); }
        bool isOpen() const { return _handle != INVALID_HANDLE_VALUE; }

      private:
        void close() {
            if (isOpen()) { CloseHandle(_handle); _handle = INVALID_HANDLE_VALUE; }
        }
        size_t availableImpl() override {
            COMSTAT state{};
            DWORD errors = 0;
            return ClearCommError(_handle, &errors, &state) ? static_cast<size_t>(state.cbInQue) : 0;
        }
        size_t readImpl(uint8_t* data, size_t size) override {
            DWORD count = 0;
            return ReadFile(_handle, data, static_cast<DWORD>(size), &count, nullptr) ? count : 0;
        }
        size_t writeImpl(const uint8_t* data, size_t size) override {
            size_t offset = 0;
            while (offset < size && running) {
                DWORD count = 0;
                if (!WriteFile(_handle, data + offset, static_cast<DWORD>(size - offset), &count, nullptr) || !count)
                    break;
                offset += count;
            }
            return offset;
        }
        HANDLE _handle = INVALID_HANDLE_VALUE;
    };
#else
    class PtyCommunication : public Communication {
      public:
        explicit PtyCommunication(int fd) : _fd(fd) {}

      private:
        size_t availableImpl() override {
            if (_offset == _size) {
                const ssize_t count = ::read(_fd, _buffer.data(), _buffer.size());
                _offset = 0;
                _size = count > 0 ? static_cast<size_t>(count) : 0;
            }
            return _size - _offset;
        }

        size_t readImpl(uint8_t* data, size_t size) override {
            const size_t count = std::min(size, _size - _offset);
            std::copy_n(_buffer.data() + _offset, count, data);
            _offset += count;
            return count;
        }

        size_t writeImpl(const uint8_t* data, size_t size) override {
            size_t offset = 0;
            while (offset < size && running) {
                const ssize_t count = ::write(_fd, data + offset, size - offset);
                if (count > 0) {
                    offset += static_cast<size_t>(count);
                } else if (count < 0 && errno == EINTR) {
                    continue;
                } else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    pollfd descriptor{_fd, POLLOUT, 0};
                    if (poll(&descriptor, 1, 100) < 0 && errno != EINTR) break;
                } else {
                    break;
                }
            }
            return offset;
        }

        int _fd;
        std::array<uint8_t, 4096> _buffer{};
        size_t _offset = 0;
        size_t _size = 0;
    };
#endif
} // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    if (argc != 2) {
        std::cerr << "Usage: TracingSerialDevice COM_PORT\n";
        return 1;
    }
    const char* port = argv[1];
    SerialCommunication communication(port);
    if (!communication.isOpen()) {
        std::cerr << "Failed to open " << port << ": Windows error " << GetLastError() << '\n';
        return 1;
    }
#else
    (void)argc;
    (void)argv;
    int master = -1;
    int slave = -1;
    if (openpty(&master, &slave, nullptr, nullptr, nullptr) != 0) {
        std::cerr << "openpty: " << std::strerror(errno) << '\n';
        return 1;
    }

    const char* port = ttyname(slave);
    termios settings{};
    const int flags = fcntl(master, F_GETFL);
    if (port == nullptr || tcgetattr(slave, &settings) != 0 || flags < 0 ||
        fcntl(master, F_SETFL, flags | O_NONBLOCK) != 0) {
        std::cerr << "Failed to configure pseudo-terminal: " << std::strerror(errno) << '\n';
        close(slave);
        close(master);
        return 1;
    }
    cfmakeraw(&settings);
    if (tcsetattr(slave, TCSANOW, &settings) != 0) {
        std::cerr << "tcsetattr: " << std::strerror(errno) << '\n';
        close(slave);
        close(master);
        return 1;
    }

    PtyCommunication communication(master);
#endif
    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);
    IntegralMotions::Tracing::TraceController::init(communication);
    auto& controller = IntegralMotions::Tracing::TraceController::get();

    uint8_t counter = 0;
    float currentU = 0.0f;
    float currentV = 0.0f;
    float currentW = 0.0f;
    controller.addVariable("current_u", &currentU);
    controller.addVariable("current_v", &currentV);
    controller.addVariable("current_w", &currentW);
    controller.addVariable("counter", &counter);

    std::cout << "Serial port: " << port << "\n"
              << "Connect your tracing host, then request config and start tracing. Ctrl-C to stop.\n" << std::flush;

    using Clock = std::chrono::steady_clock;
    constexpr double sampleRate = 120000.0;
    constexpr double motorFrequency = 50.0;
    constexpr double twoPi = 6.28318530717958647692;
    constexpr double phaseOffset = twoPi / 3.0;
    const auto start = Clock::now();
    uint64_t sample = 0;
    while (running) {
        const auto elapsed = std::chrono::duration<double>(Clock::now() - start).count();
        const auto targetSample = static_cast<uint64_t>(elapsed * sampleRate);
        if (sample <= targetSample) {
            // Skip missed desktop deadlines rather than attempting an unbounded catch-up.
            // The counter follows simulated interrupt ticks, including skipped samples.
            const uint64_t elapsedSamples = targetSample - sample + 1;
            sample = targetSample + 1;
            counter = static_cast<uint8_t>(counter + elapsedSamples);
            const double phase = twoPi * motorFrequency * static_cast<double>(targetSample % 2400) / sampleRate;
            currentU = static_cast<float>(10.0 * std::sin(phase));
            currentV = static_cast<float>(10.0 * std::sin(phase - phaseOffset));
            currentW = static_cast<float>(10.0 * std::sin(phase + phaseOffset));
        }
        controller.loop();
    }

    IntegralMotions::Tracing::TraceController::shutdown();
#ifndef _WIN32
    close(slave);
    close(master);
#endif
}
