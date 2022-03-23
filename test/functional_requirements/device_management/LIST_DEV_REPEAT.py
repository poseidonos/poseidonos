#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
import json_parser
import pos
import cli
import api
import SCAN_DEV_BASIC

def check_result(out1, out2):
    list1 = json.loads(out1)['Response']['result']['data']['devicelist']
    list2 = json.loads(out2)['Response']['result']['data']['devicelist']

    if list1 == list2:
        return "pass"
    return "fail"

def execute():
    SCAN_DEV_BASIC.execute()
    out1 = cli.list_device()
    out2 = cli.list_device()
    return out1, out2

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out1, out2 = execute()
    result = check_result(out1, out2)
    ret = api.set_result_manually(cli.list_device(), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)