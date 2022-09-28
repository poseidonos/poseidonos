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
import MOUNT_VOL_ON_RAID10_ARRAY
import fio
import time
DETACH_TARGET_DEV = "unvme-ns-0"
NEW_SPARE_DEV = "unvme-ns-4"
ARRAYNAME = MOUNT_VOL_ON_RAID10_ARRAY.ARRAYNAME


def execute():
    MOUNT_VOL_ON_RAID10_ARRAY.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV)
    time.sleep(1)
    cli.add_device(NEW_SPARE_DEV, ARRAYNAME)
    time.sleep(1)
    timeout = 80000 #80secs
    if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
        print ("now rebuilding...")
        if api.wait_situation(ARRAYNAME, "NORMAL", timeout) == True:
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