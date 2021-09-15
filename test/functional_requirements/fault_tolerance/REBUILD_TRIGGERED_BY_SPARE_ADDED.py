#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import DETACH_DEV_DATA_AND_DEGRADED_IO
import fio
import time

DETACH_TARGET_DEV = DETACH_DEV_DATA_AND_DEGRADED_IO.DETACH_TARGET_DEV
NEW_SPARE = DETACH_DEV_DATA_AND_DEGRADED_IO.REMAINING_DEV
ARRAYNAME = DETACH_DEV_DATA_AND_DEGRADED_IO.ARRAYNAME

def execute():
    DETACH_DEV_DATA_AND_DEGRADED_IO.execute()
    out = cli.add_device(NEW_SPARE, ARRAYNAME)
    timeout = 80000 #80s
    if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
        if api.wait_situation(ARRAYNAME, "NORMAL") == True:
            return "pass"
    return "fail"

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)