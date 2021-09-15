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
import UNMOUNT_ARRAY_WITH_VOL_MOUNTED

ARRAYNAME = UNMOUNT_ARRAY_WITH_VOL_MOUNTED.ARRAYNAME

def execute():
    out = UNMOUNT_ARRAY_WITH_VOL_MOUNTED.execute()
    ret = json_parser.get_response_code(out)
    if ret == 0:
        out = cli.delete_array(UNMOUNT_ARRAY_WITH_VOL_MOUNTED.ARRAYNAME)
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