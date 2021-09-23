#!/usr/bin/env python3

import socket
import sys

host = sys.argv[1]
port = sys.argv[2]

print(f"[*] Connecting to {host}:{port}")

readSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
readSocket.connect ((host, int(port)))

try:
    while (True):
        print(readSocket.recv(4096).decode("utf-8"))
except KeyboardInterrupt:
    sys.exit(0)