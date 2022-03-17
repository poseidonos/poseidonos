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
import MOUNT_RAID10_ARRAY_NO_SPARE

ARRAYNAME = MOUNT_RAID10_ARRAY_NO_SPARE.ARRAYNAME
SPARE = "unvme-ns-3"

def check_result():
    if api.is_spare_device(ARRAYNAME, SPARE) == True:
        return "pass"
    return "fail"

def execute():
    MOUNT_RAID10_ARRAY_NO_SPARE.execute()
    out = cli.add_device(SPARE, ARRAYNAME)
    print (out)
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