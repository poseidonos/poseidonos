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
import LIST_DEV_BASIC

SMART_TARGET_DEV = "unvme-ns-0"


def execute():
    out = LIST_DEV_BASIC.execute()
    if len(out) != 0:
        out = cli.smart(SMART_TARGET_DEV)
        return out
    return "fail"


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
