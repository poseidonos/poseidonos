#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../device/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import DETACH_DEV_DATA_AND_DEGRADED_IO
import fio
import time

DETACH_TARGET_DEV = DETACH_DEV_DATA_AND_DEGRADED_IO.DETACH_TARGET_DEV
NEW_SPARE = DETACH_DEV_DATA_AND_DEGRADED_IO.REMAINING_DEV
ARRAYNAME  DETACH_DEV_DATA_AND_DEGRADED_IO.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    state = json_parser.get_state(out)
    if state == "NORMAL":
        data = json.loads(out)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", out
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()
    code = json_parser.get_response_code(out)

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    DETACH_DEV_DATA_AND_DEGRADED_IO.execute()
    out = cli.add_device(NEW_SPARE, ARRAYNAME)
    rebuild_started = False
    while True:
        out = cli.array_info(ARRAYNAME)
        situ = json_parser.get_situation(out)
        if situ.find("REBUILD") == -1 and rebuild_started == True:
            print ("rebuilding done")
            break
        elif rebuild_started == False and situ.find("REBUILD") >= 0:
            print ("rebuilding started")
            rebuild_started = True
        time.sleep(1)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()