#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA


def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "NORMAL":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", list
        return "pass", out
    return "fail", out

def set_result():
    detail = cli.get_pos_info()
    code = json_parser.get_response_code(detail)
    if code == 0:
        result, out = check_result()
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 120)
    time.sleep(10)
    pos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(0.1)
    rebuild_started = False
    while True:
        out = cli.get_pos_info()
        if out.find("REBUILD") == -1 and rebuild_started == True:
            print ("rebuilding done")
            fio.wait_fio(fio_proc)
            break
        elif rebuild_started == False and out.find("REBUILD") >= 0:
            print ("rebuilding started")
            rebuild_started = True
        time.sleep(1)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()