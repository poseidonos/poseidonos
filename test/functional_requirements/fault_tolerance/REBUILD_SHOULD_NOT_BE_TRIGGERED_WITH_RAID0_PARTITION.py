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
import MOUNT_VOL_ON_RAID0_ARRAY
import fio
import time
DETACH_TARGET_DEV = "unvme-ns-0"
ARRAYNAME = MOUNT_VOL_ON_RAID0_ARRAY.ARRAYNAME

def execute():
    MOUNT_VOL_ON_RAID0_ARRAY.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd(DETACH_TARGET_DEV)
    timeout = 10000 #10secs
    if api.wait_situation(ARRAYNAME, "FAULT", timeout) == True:
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