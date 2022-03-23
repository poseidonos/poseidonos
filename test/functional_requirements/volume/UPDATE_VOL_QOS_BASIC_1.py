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
import MOUNT_VOL_BASIC_1
import volume

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME
NAME = MOUNT_VOL_BASIC_1.VOL_NAME
SIZE = MOUNT_VOL_BASIC_1.VOL_SIZE
IOPS = 100
BW = 200


def execute():
    MOUNT_VOL_BASIC_1.execute()
    out = cli.update_volume_qos(NAME, str(IOPS), str(BW), ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
