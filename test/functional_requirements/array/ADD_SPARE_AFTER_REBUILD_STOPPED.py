#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../fault_tolerance/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import REBUILD_STOP_BASIC

ARRAYNAME = REBUILD_STOP_BASIC.ARRAYNAME

def execute():
    REBUILD_STOP_BASIC.execute()
    api.rescan_ssd()
    out = cli.add_device(REBUILD_STOP_BASIC.REMAINING_DEV, ARRAYNAME)
    api.wait_situation(ARRAYNAME, "REBUILDING")
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)