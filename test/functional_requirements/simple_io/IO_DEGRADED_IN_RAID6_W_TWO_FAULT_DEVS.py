#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")
sys.path.append("../volume/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import fio
import MOUNT_VOL_ON_RAID6_ARRAY
ARRAYNAME = MOUNT_VOL_ON_RAID6_ARRAY.ARRAYNAME

# DETACH_TARGET must be a device that is not mirror-related to each other since META partition still in RAID10
DETACH_TARGET1 = "unvme-ns-0"
DETACH_TARGET2 = "unvme-ns-1"


def execute():
    MOUNT_VOL_ON_RAID6_ARRAY.execute()
    fio_proc = fio.start_fio(0, 50)
    time.sleep(10)
    print("detach device: " + DETACH_TARGET1)
    api.detach_ssd(DETACH_TARGET1)
    time.sleep(1)
    print("detach device: " + DETACH_TARGET2)
    api.detach_ssd(DETACH_TARGET2)
    fio.wait_fio(fio_proc)
    return cli.array_info(ARRAYNAME)


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_situation_eq(ARRAYNAME, out, "DEGRADED", __file__)
    pos.flush_and_kill_pos()
    exit(ret)
