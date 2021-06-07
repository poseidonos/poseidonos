#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../device_management/")

import json_parser
import pos
import cli
import api
import SCAN_DEV_BASIC
import array_device

def execute():
    SCAN_DEV_BASIC.execute()
    cli.mbr_reset()
    out = cli.list_array_device("TEMPARRAY")
    return out

if __name__ == "__main__":
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 2522, __file__)
    pos.kill_pos()
    exit(ret)