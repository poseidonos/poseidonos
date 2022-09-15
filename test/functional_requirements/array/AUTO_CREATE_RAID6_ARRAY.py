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

ARRAYNAME = "POSArray"


def execute():
    SCAN_DEV_BASIC.execute()
    cli.mbr_reset()
    out = cli.auto_create_array("uram0", 4, 0, ARRAYNAME, "RAID6")
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    print(cli.array_info(ARRAYNAME))
    pos.flush_and_kill_pos()
    exit(ret)
