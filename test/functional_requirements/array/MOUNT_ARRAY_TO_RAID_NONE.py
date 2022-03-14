#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import api
import json
import CREATE_ARRAY_TO_RAID_NONE_W_SINGLE_DEV

ARRAYNAME = CREATE_ARRAY_TO_RAID_NONE_W_SINGLE_DEV.ARRAYNAME

def execute():
    CREATE_ARRAY_TO_RAID_NONE_W_SINGLE_DEV.execute()
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
