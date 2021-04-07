#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../device/")

import json_parser
import ibofos
import ibofos_util
import cli
import test_result
import json
import DETACH_DEV_DATA_DEGRADED_IO
import fio
import time

DETACH_TARGET_DEV = DETACH_DEV_DATA_DEGRADED_IO.DETACH_TARGET_DEV
NEW_SPARE = DETACH_DEV_DATA_DEGRADED_IO.REMAINING_DEV

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['situation'] == "NORMAL":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", list
        return "pass", out
    return "fail", out

def set_result():
    detail = cli.get_ibofos_info()
    code = json_parser.get_response_code(detail)
    if code == 0:
        result, out = check_result()
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    DETACH_DEV_DATA_DEGRADED_IO.execute()
    out = cli.add_device(NEW_SPARE, "")
    rebuild_started = False
    while True:
        out = cli.get_ibofos_info()
        if out.find("REBUILDING") == -1 and rebuild_started == True:
            print ("rebuilding done")
            break
        elif rebuild_started == False and out.find("REBUILDING") >= 0:
            print ("rebuilding started")
            rebuild_started = True
        time.sleep(1)

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()