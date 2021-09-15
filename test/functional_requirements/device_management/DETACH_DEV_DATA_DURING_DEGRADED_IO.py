#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import pos_constant
import json_parser
import pos
import pos_util
import cli
import api
import json
import fio
import time
import DETACH_DEV_DATA_DURING_IO

DETACH_TARGET = DETACH_DEV_DATA_DURING_IO.ANY_OTHER_DATA
ARRAYNAME = DETACH_DEV_DATA_DURING_IO.ARRAYNAME

def check_result():
    if api.check_situation(ARRAYNAME, "FAULT") == True:
        return "pass"
    return "fail"

def execute():
    DETACH_DEV_DATA_DURING_IO.execute()
    api.detach_ssd(DETACH_TARGET)

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    execute()
    result = check_result()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)