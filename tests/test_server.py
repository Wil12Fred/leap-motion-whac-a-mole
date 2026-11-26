#!/usr/bin/env python3
"""Protocol test for the real ./server over TCP (no Leap SDK needed).

Two clients join one game, the way client.cc talks to the server:
  0001       join      -> "00010" (first: collaborator) / "00011" (second: killer)
  0002       how many  -> "0002" + 4-digit length + count        (9 moles)
  0003       board     -> "0003" + "0009" + one 0/1 per mole
  0004LLLLn  toggle mole n -> the other player gets "0005" + 2-digit n
  0005       leave     -> the other player gets "0007"
Once the killer joins, the server also pushes "00080001n" (mole n blocked)
at random, so the reader below parses messages by type instead of expecting
a fixed byte sequence.
"""
import os
import socket
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
SERVER = os.path.join(HERE, "..", "server")


class Conn:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port), timeout=5)
        self.buf = b""

    def send(self, data):
        self.s.sendall(data.encode())
        time.sleep(0.2)  # the server reads fixed-size chunks; keep messages apart

    def take(self, n):
        while len(self.buf) < n:
            chunk = self.s.recv(4096)
            if not chunk:
                raise AssertionError("connection closed")
            self.buf += chunk
        out, self.buf = self.buf[:n].decode(), self.buf[n:]
        return out

    def message(self):
        """Next message that is not a random 'mole blocked' push."""
        while True:
            head = self.take(4)
            if head == "0008":
                self.take(5)
                continue
            if head == "0001":
                return head + self.take(1)
            if head in ("0002", "0003"):
                size = self.take(4)
                return head + size + self.take(int(size))
            if head == "0005":
                return head + self.take(2)
            return head  # 0006 / 0007

    def try_message(self, wait):
        self.s.settimeout(wait)
        try:
            return self.message()
        except (socket.timeout, TimeoutError):
            return None
        finally:
            self.s.settimeout(5)


def main():
    proc = subprocess.Popen([SERVER], stdout=subprocess.PIPE, text=True)
    try:
        port = None
        while port is None:
            line = proc.stdout.readline()
            if not line:
                raise AssertionError("server exited before listening")
            if "connection success" in line:
                port = int(line.split()[1].rstrip(":"))
        print("server listening on port", port)

        c1 = Conn(port)
        assert c1.take(12) == "0007RWelcome", "welcome message"
        c1.send("0001")
        assert c1.message() == "00010", "first player is the collaborator"
        c1.send("0002")
        assert c1.message() == "000200019", "nine moles"
        c1.send("0003")
        assert c1.message() == "00030009000000000", "all moles down"
        print("ok: collaborator joins, 9 moles, all down")

        c2 = Conn(port)
        assert c2.take(12) == "0007RWelcome"
        c2.send("0001")
        assert c2.message() == "00011", "second player is the killer"
        print("ok: second player joins as the killer")

        # the collaborator raises mole 3: the killer is told, unless the server
        # blocked that mole in between, in which case the move bounces back
        c1.send("000400013")
        to_killer = c2.try_message(2)
        bounced = c1.try_message(0.5) if to_killer is None else None
        assert "000503" in (to_killer, bounced), (to_killer, bounced)
        c1.send("0003")
        board = c1.message()
        expected = "1" if to_killer == "000503" else "0"
        assert board.startswith("00030009") and board[8 + 3] == expected, board
        print("ok: raising mole 3 reaches the killer and shows on the board")

        c1.send("0005")
        assert c2.message() == "0007", "killer is told the collaborator left"
        print("ok: leaving ends the game for the other player")
    finally:
        proc.kill()
        proc.wait()


if __name__ == "__main__":
    try:
        main()
    except AssertionError as e:
        print("FAIL:", e)
        sys.exit(1)
