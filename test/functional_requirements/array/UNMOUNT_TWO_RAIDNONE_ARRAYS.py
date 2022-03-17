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
import MOUNT_TWO_RAIDNONE_ARRAYS

ARRAY1NAME = MOUNT_TWO_RAIDNONE_ARRAYS.ARRAY1NAME
ARRAY2NAME = MOUNT_TWO_RAIDNONE_ARRAYS.ARRAY2NAME

def execute():
    MOUNT_TWO_RAIDNONE_ARRAYS.execute()
    out = cli.unmount_array(ARRAY1NAME)
    print (out)
    if json_parser.get_response_code(out) is 0:
        out = cli.unmount_array(ARRAY2NAME)
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