#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import json_parser
import pos
import cli
import api
import CREATE_TWO_RAIDNONE_ARRAYS

URAM1 = CREATE_TWO_RAIDNONE_ARRAYS.URAM1
URAM2 = CREATE_TWO_RAIDNONE_ARRAYS.URAM2
DATA1 = CREATE_TWO_RAIDNONE_ARRAYS.DATA1
DATA2 = CREATE_TWO_RAIDNONE_ARRAYS.DATA2
ARRAY1NAME = CREATE_TWO_RAIDNONE_ARRAYS.ARRAY1NAME
ARRAY2NAME = CREATE_TWO_RAIDNONE_ARRAYS.ARRAY2NAME

def execute():
    CREATE_TWO_RAIDNONE_ARRAYS.execute()
    out = cli.create_array("uram3", "unvme-ns-2", "", "POSArray3", "NONE")
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)