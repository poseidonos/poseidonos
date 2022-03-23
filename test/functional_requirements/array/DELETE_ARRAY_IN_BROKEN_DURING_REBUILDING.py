#!/usr/bin/env python3
import subprocess
import os
import sys
import time

sys.path.append("../")
sys.path.append("../volume/")
sys.path.append("../../system/lib/")

import api
import cli
import pos
import json_parser
import fio
import MOUNT_VOL_BASIC_1

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME


def execute():
    api.rescan_ssd()
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    api.detach_ssd(MOUNT_VOL_BASIC_1.ANY_DATA)
    api.detach_ssd("unvme-ns-1")
    time.sleep(1)
    out = cli.delete_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
