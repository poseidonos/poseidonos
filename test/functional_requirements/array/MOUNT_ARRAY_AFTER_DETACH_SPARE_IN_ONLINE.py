#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import api
import json
import time
import pos_util
import MOUNT_ARRAY_BASIC

ARRAYNAME = MOUNT_ARRAY_BASIC.ARRAYNAME
SPARE = MOUNT_ARRAY_BASIC.SPARE

def check_result(out):
    if api.is_online(ARRAYNAME) == True:
        if api.is_spare_device(ARRAYNAME, SPARE) == False:
            return "pass"
    return "fail"

def execute():
    MOUNT_ARRAY_BASIC.execute()
    api.detach_ssd(MOUNT_ARRAY_BASIC.SPARE)
    cli.unmount_array(ARRAYNAME)
    cli.mount_array(ARRAYNAME)
    out = cli.array_info(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)