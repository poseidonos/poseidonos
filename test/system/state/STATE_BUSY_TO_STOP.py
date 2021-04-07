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
import time
import DETACH_DEV_DATA

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "STOP":
        return "pass"
    return "fail"

def set_result():
    result = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n")

def execute():
    clear_result()
    out = DETACH_DEV_DATA.execute()
    time.sleep(1)
    ibofos_util.pci_detach(DETACH_DEV_DATA.DETACH_ANOTHER_TARGET_DEV)
    time.sleep(0.1)

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()