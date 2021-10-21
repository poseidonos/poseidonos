#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import fio
import MOUNT_ARRAY_BASIC

ARRAYNAME = MOUNT_ARRAY_BASIC.ARRAYNAME

def execute():
    MOUNT_ARRAY_BASIC.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd(MOUNT_ARRAY_BASIC.ANY_DATA)
    timeout = 5000
    api.wait_situation(ARRAYNAME, "REBUILDING", timeout)
    out = cli.unmount_array(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
