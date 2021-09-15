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

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(expected):
    out = cli.array_info(ARRAYNAME)
    used = json_parser.get_used(out)
    result = "fail"
    if used != 0 and used == expected:
        result = "pass"
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (0)" + "\n" + out)

def execute():
    clear_result()
    MOUNT_VOL_BASIC_1.execute()
    out = cli.array_info(ARRAYNAME)
    used = json_parser.get_used(out)
    cli.unmount_array(MOUNT_VOL_BASIC_1.ARRAYNAME)
    cli.mount_array(MOUNT_VOL_BASIC_1.ARRAYNAME)
    return used

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    set_result(out)
    pos.flush_and_kill_pos()