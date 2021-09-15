#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")

import pos
import cli
import api
import EXIT_POS_AFTER_UNMOUNT_VOL

POS_ROOT = '../../../'


def check_result(out):
    data = json.loads(out)
    description = data['Response']['result']['status']['description']
    if "There is no array" in description:
        return "pass"
    return "fail"


def execute():
    EXIT_POS_AFTER_UNMOUNT_VOL.execute()
    ibofos_mbr_reset = POS_ROOT + "/test/script/mbr_reset.sh"
    subprocess.call([ibofos_mbr_reset])
    pos.start_pos()
    cli.scan_device()
    out = cli.list_array()
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
    