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
import MOUNT_ARRAY_BASIC
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME
DATA = MOUNT_ARRAY_BASIC.ANY_OTHER_DATA

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 15)
    fio.wait_fio(fio_proc)
    api.detach_ssd(DETACH_TARGET_DEV)
    time.sleep(1)
    out = cli.replace_device(DATA, ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
