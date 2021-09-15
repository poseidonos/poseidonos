#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import SCAN_DEV_BASIC

import json_parser
import pos
import cli
import api

def execute():
    SCAN_DEV_BASIC.execute()
    cli.mbr_reset()
    cli.create_array("uram0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "", "POS1", "RAID5")
    out = cli.create_array("uram0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "", "POS2", "RAID5")
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)