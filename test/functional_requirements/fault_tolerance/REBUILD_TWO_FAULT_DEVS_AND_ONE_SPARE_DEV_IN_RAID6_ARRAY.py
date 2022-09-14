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
DETACH_TARGET_DEV1 = "unvme-ns-0"
DETACH_TARGET_DEV2 = "unvme-ns-1"
ARRAYNAME = MOUNT_VOL_ON_RAID6_ARRAY.ARRAYNAME
SPARE_DEV_1 = "unvme-ns-4"
SPARE_DEV_2 = "unvme-ns-5"


def execute():
    MOUNT_VOL_ON_RAID6_ARRAY.execute()
    print (api.set_rebuild_autostart("false"))
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV1)
    time.sleep(1)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV2)
    if api.wait_situation(ARRAYNAME, "DEGRADED") == True:
        time.sleep(2)
        cli.add_device(SPARE_DEV_1, ARRAYNAME)
        out = cli.rebuild_array(ARRAYNAME)
        print (out)
        timeout = 80000 #80s
        if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
            if api.wait_situation(ARRAYNAME, "DEGRADED") == True:
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
