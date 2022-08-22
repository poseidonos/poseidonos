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
import time
import MOUNT_RAID0_ARRAY

ARRAYNAME = MOUNT_RAID0_ARRAY.ARRAYNAME
DATA = MOUNT_RAID0_ARRAY.ANY_DATA


def execute():
    MOUNT_RAID0_ARRAY.execute()
    out = cli.replace_device(DATA, ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)