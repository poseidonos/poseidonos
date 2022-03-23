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
import MOUNT_ARRAY_BASIC
import volume

VOL_NAME = "vol1"
VOL_SIZE = pos_constant.SIZE_1GB
VOL_IOPS = 0
VOL_BW = 0

SPARE = MOUNT_ARRAY_BASIC.SPARE
ANY_DATA = MOUNT_ARRAY_BASIC.ANY_DATA


def execute():
    MOUNT_ARRAY_BASIC.execute()
    out = cli.create_volume(VOL_NAME, str(VOL_SIZE), "", "", "wrong_array_name")
    print(out)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)