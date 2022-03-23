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
import MOUNT_VOL_NO_SPARE
import fio
import time
import CREATE_ARRAY_NO_SPARE
DETACH_TARGET_DEV = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
ANY_OTHER_DATA = CREATE_ARRAY_NO_SPARE.DATA_DEV_2
ARRAYNAME = MOUNT_VOL_NO_SPARE.ARRAYNAME

def check_result():
    if api.check_situation(ARRAYNAME, "DEGRADED") == True:
        if api.is_device_in_the_array(ARRAYNAME, DETACH_TARGET_DEV) == False:
            return "pass"
    return "fail"

def execute():
    MOUNT_VOL_NO_SPARE.execute()
    fio_proc = fio.start_fio(0, 30)
    time.sleep(1)
    api.detach_ssd(DETACH_TARGET_DEV)
    return fio_proc

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    fio_proc = execute()
    fio.wait_fio(fio_proc)
    result = check_result()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)