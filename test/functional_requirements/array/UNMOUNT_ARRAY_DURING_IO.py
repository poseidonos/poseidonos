#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")

import json_parser
import pos
import cli
import api
import json
import MOUNT_VOL_BASIC_1
import fio
import time

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    time.sleep(5)
    out = cli.unmount_array(ARRAYNAME)
    fio.stop_fio(fio_proc)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_state_eq(ARRAYNAME, out, "OFFLINE", __file__)
    pos.flush_and_kill_pos()
    exit(ret)