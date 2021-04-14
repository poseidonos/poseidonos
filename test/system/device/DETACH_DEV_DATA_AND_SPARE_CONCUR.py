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
import time
import MOUNT_VOL_BASIC_1

DETACH_TARGET_DATA = MOUNT_VOL_BASIC_1.ANY_DATA
DETACH_TARGET_SPARE = MOUNT_VOL_BASIC_1.SPARE

def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "DEGRADED":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DATA or item['name'] == DETACH_TARGET_SPARE:
                return "fail", list
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n" + out)

def execute():
    pos_util.pci_rescan()
    MOUNT_VOL_BASIC_1.execute()
    pos_util.pci_detach(DETACH_TARGET_DATA)
    pos_util.pci_detach(DETACH_TARGET_SPARE)
    time.sleep(0.1)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()