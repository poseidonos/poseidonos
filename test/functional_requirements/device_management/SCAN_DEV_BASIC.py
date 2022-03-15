#!/usr/bin/env python3
'''

Author: SRM
'''

import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")

import json_parser
import pos
import cli
import api
import START_POS_BASIC

def execute(isSingleArray = True):
    START_POS_BASIC.execute(isSingleArray)
    out = cli.scan_device()
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)