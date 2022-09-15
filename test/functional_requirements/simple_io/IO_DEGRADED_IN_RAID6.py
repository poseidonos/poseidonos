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
DETACH_TARGET = "unvme-ns-0"


def execute():
    MOUNT_VOL_ON_RAID6_ARRAY.execute()
    fio_proc = fio.start_fio(0, 40)
    time.sleep(20)
    print("detach device: " + DETACH_TARGET)
    api.detach_ssd(DETACH_TARGET)
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
