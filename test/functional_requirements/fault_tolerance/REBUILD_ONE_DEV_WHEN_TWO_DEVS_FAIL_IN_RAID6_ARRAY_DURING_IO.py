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
import MOUNT_VOL_ON_RAID6_ARRAY
import fio
import time
DETACH_TARGET1 = "unvme-ns-0"
DETACH_TARGET2 = "unvme-ns-1"
ARRAYNAME = MOUNT_VOL_ON_RAID6_ARRAY.ARRAYNAME
SPARENAME = "unvme-ns-4"


def execute():
    MOUNT_VOL_ON_RAID6_ARRAY.execute()
    fio_proc = fio.start_fio(0, 90)
    time.sleep(10)
    api.detach_ssd_and_attach(DETACH_TARGET1)
    time.sleep(2)
    api.detach_ssd_and_attach(DETACH_TARGET2)
    if api.wait_situation(ARRAYNAME, "DEGRADED") == True:
        time.sleep(3)
        out = cli.add_device(SPARENAME, ARRAYNAME)
        print (out)
        timeout = 80000 #80s
        if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
            if api.wait_situation(ARRAYNAME, "DEGRADED") == True:
                fio.wait_fio(fio_proc)
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
