#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")
sys.path.append("../device_management/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import SCAN_DEV_BASIC
import MOUNT_VOL_BASIC_1
import fio
import time
REPLACE_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME


def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    cli.replace_device(REPLACE_TARGET_DEV, ARRAYNAME)
    timeout = 10000
    if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
        print("Quick Rebuild started")
        pos.kill_pos()
        print("SPOR")
        uram_backup = "../../../script/backup_latest_hugepages_for_uram.sh"
        subprocess.call([uram_backup])
        time.sleep(5)
        pos.start_pos()
        cli.scan_device()
        cli.mount_array(ARRAYNAME)
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
