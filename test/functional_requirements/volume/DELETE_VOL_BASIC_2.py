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
import UNMOUNT_VOL_BASIC_1

VOL_NAME = UNMOUNT_VOL_BASIC_1.VOL_NAME
ARRAYNAME = UNMOUNT_VOL_BASIC_1.ARRAYNAME


def execute():
    UNMOUNT_VOL_BASIC_1.execute()
    out = cli.delete_volume(VOL_NAME, ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)