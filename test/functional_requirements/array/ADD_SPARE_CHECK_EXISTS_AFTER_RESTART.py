#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import json
import ADD_SPARE_BASIC

SPARE_DEV = ADD_SPARE_BASIC.SPARE_DEV
ARRAYNAME = ADD_SPARE_BASIC.ARRAYNAME

def check_result():
    if api.is_spare_device(ARRAYNAME, SPARE_DEV) == True:
        return "pass"
    return "fail"

def execute():
    ADD_SPARE_BASIC.execute()
    cli.unmount_array(ARRAYNAME)
    pos.exit_pos()
    pos.start_pos()
    cli.scan_device()
    cli.mount_array(ARRAYNAME)
    out = cli.array_info(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result()
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)