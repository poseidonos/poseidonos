#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import json
import UNMOUNT_ARRAY_WITH_VOL_MOUNTED

ARRAYNAME = UNMOUNT_ARRAY_WITH_VOL_MOUNTED.ARRAYNAME

def execute():
    UNMOUNT_ARRAY_WITH_VOL_MOUNTED.execute()
    out = pos.exit_pos()
    return out

if __name__ == "__main__":
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.kill_pos()
    exit(ret)