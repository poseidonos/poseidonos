#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import ibofos
import ibofos_util
import cli
import test_result
import json
import time
import MOUNT_VOL_BASIC_1
import MOUNT_ARRAY_BASIC_1
DETACH_TARGET_DEV = MOUNT_ARRAY_BASIC_1.SPARE

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
    result, out = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n" + out)

def execute():
    clear_result()
    MOUNT_VOL_BASIC_1.execute()
    ibofos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(0.1)

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()