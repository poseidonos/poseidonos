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
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 200)
    time.sleep(10)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV)
    if api.wait_situation(ARRAYNAME, "REBUILDING") == True:
        print ("1st rebuilding has been started")
        time.sleep(1)
        rebuilding_target_device = MOUNT_VOL_BASIC_1.SPARE
        api.detach_ssd(rebuilding_target_device)
        print ("wait for 1st rebuilding stopped")
        timeout = 80000 #80s
        if api.wait_situation(ARRAYNAME, "DEGRADED", timeout) == True: #wait for 1st rebuilding done
            print ("1st rebuilding has been stopped")
            spare_dev_newly_attached = "unvme-ns-4"
            print (cli.add_device(spare_dev_newly_attached, ARRAYNAME))
            if api.wait_situation(ARRAYNAME, "REBUILDING") == True: #wait for 2nd rebuilding
                print ("2nd rebuilding has been started")
                if api.wait_situation(ARRAYNAME, "NORMAL") == True: #wait for 2nd rebuilding done
                    print ("2nd rebuilding complete")
                    fio.stop_fio(fio_proc)
                    return "pass"
        fio.stop_fio(fio_proc)
        return "fail"

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)