#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import time
import MOUNT_ARRAY_NO_SPARE
ARRAYNAME = MOUNT_ARRAY_NO_SPARE.ARRAYNAME

def execute():
    MOUNT_ARRAY_NO_SPARE.execute()
    api.detach_ssd(MOUNT_ARRAY_NO_SPARE.DATA_DEV_1)
    api.detach_ssd(MOUNT_ARRAY_NO_SPARE.DATA_DEV_2)
    cli.unmount_array(ARRAYNAME)
    out = cli.mount_array(ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_state_eq(ARRAYNAME, out, "STOP", __file__)
    pos.flush_and_kill_pos()
    exit(ret)