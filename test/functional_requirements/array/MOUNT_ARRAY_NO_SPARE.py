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
DATA_DEV_1 = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
DATA_DEV_2 = CREATE_ARRAY_NO_SPARE.DATA_DEV_2
ANY_DATA = DATA_DEV_1
REMAINING_DEV = CREATE_ARRAY_NO_SPARE.REMAINING_DEV

def execute():
    CREATE_ARRAY_NO_SPARE.execute()
    out = cli.mount_array(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
