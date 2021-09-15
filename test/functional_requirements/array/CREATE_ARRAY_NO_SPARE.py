#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import json_parser
import pos
import cli
import api
import SCAN_DEV_BASIC

DATA_DEV_1 = "unvme-ns-0"
DATA_DEV_2 = "unvme-ns-1"
DATA_DEV_3 = "unvme-ns-2"
REMAINING_DEV = "unvme-ns-3"
ARRAYNAME = "POSArray"


def execute():
    SCAN_DEV_BASIC.execute()
    cli.mbr_reset()
    out = cli.create_array("uram0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "", ARRAYNAME, "RAID5")
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)