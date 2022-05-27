#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import MOUNT_VOL_NO_SPARE

REMAINING_DEV = MOUNT_VOL_NO_SPARE.REMAINING_DEV
ARRAYNAME = MOUNT_VOL_NO_SPARE.ARRAYNAME


def check_result():
    if api.is_device_exists(REMAINING_DEV) == False:
        return "pass"
    return "fail"


def execute():
    MOUNT_VOL_NO_SPARE.execute()
    print(cli.list_device())
    api.detach_ssd(REMAINING_DEV)
    time.sleep(1)


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    execute()
    result = check_result()
    ret = api.set_result_manually(cli.list_device(), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)