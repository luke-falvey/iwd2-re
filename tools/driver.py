#!/usr/bin/env python3
"""Talk to a running port build's test driver (IWD2RE_TEST_DRIVER=<port>).

  python3 -I tools/driver.py [--port 47800] grab out.png
  python3 -I tools/driver.py click X Y [--right]
  python3 -I tools/driver.py key VK
  python3 -I tools/driver.py status | close

The protocol is described in src/platform/posix/test_driver.cpp.
"""
import argparse
import socket
import struct
import sys
import time
import zlib


class Driver:
    def __init__(self, port=47800, host="127.0.0.1"):
        self.sock = socket.create_connection((host, port), timeout=30)
        self.buf = b""

    def _line(self):
        while b"\n" not in self.buf:
            data = self.sock.recv(65536)
            if not data:
                raise ConnectionError("driver closed the connection")
            self.buf += data
        line, self.buf = self.buf.split(b"\n", 1)
        return line.decode()

    def _bytes(self, n):
        while len(self.buf) < n:
            data = self.sock.recv(1 << 20)
            if not data:
                raise ConnectionError("driver closed the connection")
            self.buf += data
        out, self.buf = self.buf[:n], self.buf[n:]
        return out

    def cmd(self, text):
        self.sock.sendall(text.encode() + b"\n")
        return self._line()

    def status(self):
        _, count, w, h = self.cmd("status").split()
        return int(count), int(w), int(h)

    def grab(self):
        _, w, h, count = self.cmd("grab").split()
        w, h = int(w), int(h)
        return w, h, self._bytes(w * h * 3), int(count)

    def move(self, x, y):
        self.cmd(f"move {x} {y}")

    def click(self, x, y, button=1, hold=0.08):
        self.move(x, y)
        time.sleep(0.15)
        self.cmd(f"down {button}")
        time.sleep(hold)
        self.cmd(f"up {button}")

    def key(self, vk, hold=0.08):
        self.cmd(f"keydown {vk}")
        time.sleep(hold)
        self.cmd(f"keyup {vk}")


def write_png(path, w, h, rgb):
    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data))

    rows = b"".join(b"\x00" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(rows, 6)) + chunk(b"IEND", b""))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=47800)
    ap.add_argument("--right", action="store_true")
    ap.add_argument("command")
    ap.add_argument("args", nargs="*")
    a = ap.parse_args()

    d = Driver(a.port)
    if a.command == "grab":
        w, h, rgb, count = d.grab()
        if w == 0:
            sys.exit("no frame yet")
        write_png(a.args[0], w, h, rgb)
        print(f"{w}x{h} frame {count} -> {a.args[0]}")
    elif a.command == "click":
        d.click(int(a.args[0]), int(a.args[1]), 2 if a.right else 1)
    elif a.command == "move":
        d.move(int(a.args[0]), int(a.args[1]))
    elif a.command == "key":
        d.key(int(a.args[0], 0))
    elif a.command == "status":
        print(d.status())
    elif a.command == "close":
        print(d.cmd("close"))
    else:
        sys.exit("unknown command")


if __name__ == "__main__":
    main()
