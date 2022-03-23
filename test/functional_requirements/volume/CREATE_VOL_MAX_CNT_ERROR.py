#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import pos_constant
import CREATE_VOL_BASIC_8

ARRAYNAME = CREATE_VOL_BASIC_8.ARRAYNAME
VOL_NAME_PREFIX = "vol"
VOL_SIZE = pos_constant.SIZE_1MB * 100
VOL_IOPS = 10
VOL_BW = 10



def execute():
    CREATE_VOL_BASIC_8.execute()
    out = cli.create_volume(VOL_NAME_PREFIX + str(257), str(VOL_SIZE), str(VOL_IOPS), str(VOL_BW), ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
