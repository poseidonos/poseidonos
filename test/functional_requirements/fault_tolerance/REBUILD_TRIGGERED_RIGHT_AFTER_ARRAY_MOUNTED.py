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
import CREATE_ARRAY_BASIC
ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME

def execute():
    CREATE_ARRAY_BASIC.execute()
    api.detach_ssd(CREATE_ARRAY_BASIC.ANY_DATA)
    time.sleep(5)
    out = cli.mount_array(CREATE_ARRAY_BASIC.ARRAYNAME)
    timeout = 80000 #80s
    if api.wait_situation(ARRAYNAME, "REBUILDING", timeout) == True:
        return "pass"
    return "fail"

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)