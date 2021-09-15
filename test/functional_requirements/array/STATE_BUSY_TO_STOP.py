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
import MOUNT_ARRAY_DEGRADED_BASIC

ARRAYNAME = MOUNT_ARRAY_DEGRADED_BASIC.ARRAYNAME

def execute():
    MOUNT_ARRAY_DEGRADED_BASIC.execute()
    time.sleep(1)
    api.detach_ssd(MOUNT_ARRAY_DEGRADED_BASIC.ANY_ANOTHER_DATA)

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    execute()
    out = cli.array_info(ARRAYNAME)
    ret = api.set_result_by_situation_eq(ARRAYNAME, out, "FAULT", __file__)
    pos.flush_and_kill_pos()
    exit(ret)