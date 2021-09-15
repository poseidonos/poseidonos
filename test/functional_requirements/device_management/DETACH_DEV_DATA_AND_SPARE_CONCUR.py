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
import pos_util
import cli
import api
import json
import time
import MOUNT_VOL_BASIC_1

DETACH_TARGET_DATA = MOUNT_VOL_BASIC_1.ANY_DATA
DETACH_TARGET_SPARE = MOUNT_VOL_BASIC_1.SPARE
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def check_result():
    if api.check_situation(ARRAYNAME, "DEGRADED") == True:
        if api.is_device_in_the_array(ARRAYNAME, DETACH_TARGET_DATA) == False:
            if api.is_device_in_the_array(ARRAYNAME, DETACH_TARGET_SPARE) == False:
                return "pass"
    return "fail"

def execute():
    MOUNT_VOL_BASIC_1.execute()
    api.detach_ssd(DETACH_TARGET_DATA)
    api.detach_ssd(DETACH_TARGET_SPARE)
    time.sleep(0.1)

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    execute()
    result = check_result()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)