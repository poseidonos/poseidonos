#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")

import json_parser
import pos
import cli
import time
import api
import EXIT_POS_AFTER_UNMOUNT_VOL

POS_ROOT = '../../../'

ARRAYNAME = EXIT_POS_AFTER_UNMOUNT_VOL.ARRAYNAME


def execute():
    EXIT_POS_AFTER_UNMOUNT_VOL.execute()
    ibofos_mbr_reset = POS_ROOT + "/test/script/mbr_reset.sh"
    subprocess.call([ibofos_mbr_reset])
    pos.start_pos()
    cli.scan_device()
    out = cli.array_info(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
    