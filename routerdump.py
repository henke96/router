#!/usr/bin/env python3

# Put this file under ~/.config/wireshark/extcap/
# Then start wireshark as your user and choose the "Router capture" interface.

# TODO: Wireshark reports some TCP errors between gateway<->router when watching twitch, but the router<->PC traffic is fine. Why?

import sys
import argparse
import time
import struct
from socket import *

parser = argparse.ArgumentParser()

parser.add_argument("--capture", action="store_true" )
parser.add_argument("--extcap-interfaces", action="store_true")
parser.add_argument("--extcap-interface")  # Needed so it isn't parsed as --extcap-interfaces
parser.add_argument("--extcap-config", action="store_true")
parser.add_argument("--fifo")
parser.add_argument("--extcap-version", nargs='?', default="")

args, unknown = parser.parse_known_args()

if args.extcap_interfaces:
    print("extcap {version=1.0}{help=https://www.wireshark.org}{display=Router capture}")
    print("interface {value=routerdump}{display=Router capture}")
elif args.capture:
    sock = socket(AF_INET, SOCK_DGRAM)
    sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    sock.bind(("", 12345))

    with open(args.fifo, "wb", 0) as f:
        header = bytearray()
        header += struct.pack('<L', int("a1b2c3d4", 16))
        header += struct.pack('<H', 2)  # Pcap Major Version
        header += struct.pack('<H', 4)  # Pcap Minor Version
        header += struct.pack('<I', 0)  # Timezone
        header += struct.pack('<I', 0)  # Accuracy of timestamps
        header += struct.pack('<L', int("0000ffff", 16))  # Max Length of capture frame
        header += struct.pack('<L', 1)  # Ethernet
        f.write(header)

        while True:
            read = sock.recv(65535)

            length = len(read)
            pcap = bytearray()
            pcap += struct.pack('<L', int(time.time()))  # timestamp seconds
            pcap += struct.pack('<L', 0x00)  # timestamp nanoseconds
            pcap += struct.pack('<L', length)  # length captured
            pcap += struct.pack('<L', length)  # length in frame
            f.write(pcap)
            f.write(read)
