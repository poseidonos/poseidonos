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
import volume

ARRAYNAME = UNMOUNT_VOL_BASIC_1.ARRAYNAME
IOPS = 10
BW = 10


def execute():
    UNMOUNT_VOL_BASIC_1.execute()
    cli.delete_volume(UNMOUNT_VOL_BASIC_1.VOL_NAME, ARRAYNAME)
    out = cli.update_volume_qos(UNMOUNT_VOL_BASIC_1.VOL_NAME, str(IOPS), str(BW), ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
