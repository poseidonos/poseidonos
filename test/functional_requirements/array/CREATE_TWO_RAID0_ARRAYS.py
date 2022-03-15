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
import json_parser
import SCAN_DEV_BASIC

URAM1 = "uram0"
URAM2 = "uram1"
DATA1 = "unvme-ns-0,unvme-ns-1"
DATA2 = "unvme-ns-2,unvme-ns-3"
ARRAY1NAME = "POSArray1"
ARRAY2NAME = "POSArray2"

def execute():
    isSingleArray = False
    SCAN_DEV_BASIC.execute(isSingleArray)
    cli.mbr_reset()
    out1 = cli.create_array(URAM1, DATA1, "", ARRAY1NAME, "RAID0")
    print(out1)
    code = json_parser.get_response_code(out1)
    if code is 0:
        out2 = cli.create_array(URAM2, DATA2, "", ARRAY2NAME, "RAID0")
        print(out2)
        return out2
    else:
        return out1

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)