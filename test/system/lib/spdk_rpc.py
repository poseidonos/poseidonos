#!/usr/bin/env python3

import subprocess
import sys

IBOFOS_ROOT = '../../../'
SPDK_RPC = IBOFOS_ROOT + "lib/spdk/scripts/rpc.py"

def send_request(msg):
    out = subprocess.call(SPDK_RPC + " " + msg, universal_newlines=True, shell=True)
    return out