#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")

import json_parser
import pos
import cli
import api
import pos_constant
import CREATE_VOL_BASIC_1

ARRAYNAME = CREATE_VOL_BASIC_1.ARRAYNAME

def check_result():
    if api.get_used_size(ARRAYNAME) > 0:
        return "pass"
    return "fail"

def execute():
    CREATE_VOL_BASIC_1.execute()
    return cli.array_info(ARRAYNAME)

if __name__ == "__main__":
    api.clear_result(__file__)
    out = execute()
    result = check_result()
    ret = api.set_result_manually(out, result, __file__)
    pos.kill_pos()
    exit(ret)