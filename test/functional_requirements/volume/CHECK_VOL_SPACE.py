#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import MOUNT_VOL_BASIC_1
VOL_NAME = MOUNT_VOL_BASIC_1.VOL_NAME
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME


def check_result(expected):
    out = cli.array_info(ARRAYNAME)
    used = json_parser.get_used(out)
    if used != 0 and used == expected:
        return "pass"
    return "fail"

def execute():
    MOUNT_VOL_BASIC_1.execute()
    out = cli.array_info(ARRAYNAME)
    used = json_parser.get_used(out)
    cli.unmount_array(MOUNT_VOL_BASIC_1.ARRAYNAME)
    cli.mount_array(MOUNT_VOL_BASIC_1.ARRAYNAME)
    return used

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
