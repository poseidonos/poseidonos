#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import json
import MOUNT_ARRAY_NO_SPARE

SPARE_DEV = MOUNT_ARRAY_NO_SPARE.REMAINING_DEV
ARRAYNAME = MOUNT_ARRAY_NO_SPARE.ARRAYNAME

def check_result():
    if api.is_spare_device(ARRAYNAME, SPARE_DEV) == True:
        return "pass"
    return "fail"

def execute():
    MOUNT_ARRAY_NO_SPARE.execute()
    out = cli.add_device(SPARE_DEV, ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result()
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)