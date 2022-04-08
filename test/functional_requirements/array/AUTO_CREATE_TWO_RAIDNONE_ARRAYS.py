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
import SCAN_DEV_BASIC

ARRAY1NAME = "POSArray1"
ARRAY2NAME = "POSArray2"


def execute():
    isSingleArray = False
    SCAN_DEV_BASIC.execute(isSingleArray)
    cli.mbr_reset()
    print (cli.list_device())
    out1 = cli.auto_create_array("uram0", 1, 0, ARRAY1NAME, "NONE")
    out2 = cli.auto_create_array("uram1", 1, 0, ARRAY2NAME, "NONE")
    return out1, out2


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out1, out2 = execute()
    ret = api.set_result_by_code_eq(out1, 0, __file__)
    if ret is 0:
        ret = api.set_result_by_code_eq(out2, 0, __file__)
    print(cli.array_info(ARRAY1NAME))
    print(cli.array_info(ARRAY2NAME))
    pos.flush_and_kill_pos()
    exit(ret)
