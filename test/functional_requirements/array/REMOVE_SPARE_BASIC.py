#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import json
import MOUNT_ARRAY_BASIC
SPARE = MOUNT_ARRAY_BASIC.SPARE
ARRAYNAME = MOUNT_ARRAY_BASIC.ARRAYNAME

def check_result():
    if api.check_state(ARRAYNAME, "NORMAL") == True:
        if api.is_device_in_the_array(ARRAYNAME, SPARE) == False:
            return "pass"
    return "fail"

def execute():
    MOUNT_ARRAY_BASIC.execute()
    out = cli.remove_device(SPARE, ARRAYNAME)
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