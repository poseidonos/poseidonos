#!/usr/bin/env python3
import subprocess
import os
import sys
import time

sys.path.append("../")
sys.path.append("../../system/lib/")

import api
import cli
import pos
import json_parser
import CREATE_ARRAY_NO_SPARE

ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME


def execute():
    api.rescan_ssd()
    out = CREATE_ARRAY_NO_SPARE.execute()
    out = cli.mount_array(ARRAYNAME)
    api.detach_ssd(CREATE_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(10)
    api.detach_ssd(CREATE_ARRAY_NO_SPARE.DATA_DEV_2)
    time.sleep(10)
    out = cli.delete_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
