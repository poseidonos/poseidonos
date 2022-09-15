#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")
import json_parser
import pos
import cli
import api
import START_POS_BASIC

def check_result(out):
    data = json.loads(out)
    description = data['Response']['result']['status']['description']
    if "No device" in description:
        return "pass"
    return "fail"

def execute():
    START_POS_BASIC.execute()
    out = cli.list_device()
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)