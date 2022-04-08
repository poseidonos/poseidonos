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
DETACH_TARGET_DEV1 = "unvme-ns-0"
DETACH_TARGET_DEV2 = "unvme-ns-1"
NEW_SPARE_DEV1 = "unvme-ns-4"
NEW_SPARE_DEV2 = "unvme-ns-5"
ARRAYNAME = MOUNT_VOL_ON_RAID10_ARRAY.ARRAYNAME


def execute():
    MOUNT_VOL_ON_RAID10_ARRAY.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV1)
    time.sleep(1)
    if api.check_situation(ARRAYNAME, "DEGRADED") is False:
        return "fail"
    api.detach_ssd_and_attach(DETACH_TARGET_DEV2)
    time.sleep(1)
    if api.check_situation(ARRAYNAME, "DEGRADED") is False:
        return "fail"
    cli.add_device(NEW_SPARE_DEV1, ARRAYNAME)
    if api.wait_situation(ARRAYNAME, "REBUILDING") == True:
        print ("now 1st rebuilding...")
        newSitu = api.wait_situation_changed(ARRAYNAME, "REBUILDING")
        print ("1st rebuilding done and new situ: " + newSitu)
        if newSitu == "DEGRADED":
            print ("may be 1st rebuild complete")
            cli.add_device(NEW_SPARE_DEV2, ARRAYNAME)
            if api.wait_situation(ARRAYNAME, "REBUILDING") == True:
                print ("now 2nd rebuilding...")
                if api.wait_situation(ARRAYNAME, "NORMAL") == True:
                    print ("may be 2nd rebuild complete")
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