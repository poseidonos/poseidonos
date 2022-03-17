#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import json_parser
import pos
import cli
import api
import json_parser
import UNMOUNT_TWO_RAIDNONE_ARRAYS

ARRAY1NAME = UNMOUNT_TWO_RAIDNONE_ARRAYS.ARRAY1NAME
ARRAY2NAME = UNMOUNT_TWO_RAIDNONE_ARRAYS.ARRAY2NAME

def execute():
    UNMOUNT_TWO_RAIDNONE_ARRAYS.execute()
    pos.exit_pos()
    time.sleep(5)
    pos.start_pos_for_two_arrays()
    out = cli.scan_device()
    print (out)
    out = cli.mount_array(ARRAY1NAME)
    print (out)
    if json_parser.get_response_code(out) is 0:
        out = cli.mount_array(ARRAY2NAME)
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