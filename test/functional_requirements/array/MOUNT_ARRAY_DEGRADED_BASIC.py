#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import CREATE_ARRAY_NO_SPARE
ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME
DETACH_TARGET = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
ANY_ANOTHER_DATA = CREATE_ARRAY_NO_SPARE.DATA_DEV_2

def execute():
    CREATE_ARRAY_NO_SPARE.execute()
    api.detach_ssd(DETACH_TARGET)
    time.sleep(1)
    out = cli.mount_array(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_situation_eq(ARRAYNAME, out, "DEGRADED", __file__)
    pos.flush_and_kill_pos()
    exit(ret)