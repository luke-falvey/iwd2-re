// The test driver: lets the play tests drive the game and read its frames
// over a local socket, with no OS-level screen capture or synthetic input.
//
// Turned on by IWD2RE_TEST_DRIVER=<port>; the game then listens on
// 127.0.0.1:<port>. One client at a time. Commands are text lines; every
// command gets one reply line, and "grab" is followed by binary data:
//
//   move X Y          -> ok          mouse position in client pixels
//   down VK | up VK   -> ok          mouse button: 1 left, 2 right, 4 middle
//   keydown VK | keyup VK -> ok      Windows virtual-key code, decimal
//   wheel DELTA       -> ok          120 per notch, positive is forward
//   status            -> status COUNT W H     frames presented so far
//   grab              -> frame W H COUNT, then W*H*3 bytes of RGB, top row first
//   close             -> ok          ask the game to close (WM_CLOSE)
//
// Input goes into the same state and message queue that real input feeds
// (window_sdl.cpp). Once the driver has sent input, the real mouse and
// keyboard are ignored, so a run is not disturbed by someone using the
// machine.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <thread>
#include <vector>

namespace platform {
// From video_present.h, repeated here because that header's neighbours pull
// in the Windows socket declarations.
uint32_t GrabFrame(std::vector<uint32_t>& pixels, int& w, int& h);
void InjectMouseMove(int x, int y);
void InjectButton(int vk, bool down);
void InjectKey(int vk, bool down);
void InjectWheel(int delta);
void InjectClose();
void StartTestDriver();
}

namespace {

bool SendAll(int fd, const void* data, size_t size)
{
    const char* p = static_cast<const char*>(data);
    while (size > 0) {
        ssize_t n = send(fd, p, size, 0);
        if (n <= 0) {
            return false;
        }
        p += n;
        size -= static_cast<size_t>(n);
    }
    return true;
}

bool Reply(int fd, const std::string& line)
{
    return SendAll(fd, line.data(), line.size());
}

bool Handle(int fd, const char* line)
{
    int a = 0;
    int b = 0;
    if (sscanf(line, "move %d %d", &a, &b) == 2) {
        platform::InjectMouseMove(a, b);
        return Reply(fd, "ok\n");
    }
    if (sscanf(line, "down %d", &a) == 1) {
        platform::InjectButton(a, true);
        return Reply(fd, "ok\n");
    }
    if (sscanf(line, "up %d", &a) == 1) {
        platform::InjectButton(a, false);
        return Reply(fd, "ok\n");
    }
    if (sscanf(line, "keydown %d", &a) == 1) {
        platform::InjectKey(a, true);
        return Reply(fd, "ok\n");
    }
    if (sscanf(line, "keyup %d", &a) == 1) {
        platform::InjectKey(a, false);
        return Reply(fd, "ok\n");
    }
    if (sscanf(line, "wheel %d", &a) == 1) {
        platform::InjectWheel(a);
        return Reply(fd, "ok\n");
    }
    if (strncmp(line, "close", 5) == 0) {
        platform::InjectClose();
        return Reply(fd, "ok\n");
    }

    bool grab = strncmp(line, "grab", 4) == 0;
    if (grab || strncmp(line, "status", 6) == 0) {
        std::vector<uint32_t> pixels;
        int w = 0;
        int h = 0;
        uint32_t count = platform::GrabFrame(pixels, w, h);

        char head[64];
        if (!grab) {
            snprintf(head, sizeof(head), "status %u %d %d\n", count, w, h);
            return Reply(fd, head);
        }

        snprintf(head, sizeof(head), "frame %d %d %u\n", w, h, count);
        std::vector<uint8_t> rgb(pixels.size() * 3);
        for (size_t k = 0; k < pixels.size(); k++) {
            uint32_t c = pixels[k];
            rgb[k * 3] = static_cast<uint8_t>(c >> 16);
            rgb[k * 3 + 1] = static_cast<uint8_t>(c >> 8);
            rgb[k * 3 + 2] = static_cast<uint8_t>(c);
        }
        return Reply(fd, head) && SendAll(fd, rgb.data(), rgb.size());
    }

    return Reply(fd, "error unknown command\n");
}

void Serve(int listener)
{
    while (1) {
        int fd = accept(listener, NULL, NULL);
        if (fd < 0) {
            continue;
        }
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#ifdef SO_NOSIGPIPE
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif

        std::string pending;
        char buffer[512];
        bool ok = true;
        while (ok) {
            ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
            if (n <= 0) {
                break;
            }
            pending.append(buffer, static_cast<size_t>(n));

            size_t end;
            while (ok && (end = pending.find('\n')) != std::string::npos) {
                std::string line = pending.substr(0, end);
                pending.erase(0, end + 1);
                ok = Handle(fd, line.c_str());
            }
        }
        close(fd);
    }
}

} // namespace

void platform::StartTestDriver()
{
    const char* value = getenv("IWD2RE_TEST_DRIVER");
    if (value == NULL || atoi(value) <= 0) {
        return;
    }

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(atoi(value)));
    if (bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 || listen(listener, 1) != 0) {
        fprintf(stderr, "[driver] cannot listen on port %s\n", value);
        close(listener);
        return;
    }

    fprintf(stderr, "[driver] listening on 127.0.0.1:%s\n", value);
    std::thread(Serve, listener).detach();
}
