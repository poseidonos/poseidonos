#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import api
import json
import MOUNT_RAID6_ARRAY

ARRAYNAME = MOUNT_RAID6_ARRAY.ARRAYNAME

def execute():
    MOUNT_RAID6_ARRAY.execute()
    cli.unmount_array(ARRAYNAME)
    pos.exit_pos()
    time.sleep(5)
    pos.start_pos()
    cli.scan_device()
    out = cli.mount_array(ARRAYNAME)
    print (out)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
