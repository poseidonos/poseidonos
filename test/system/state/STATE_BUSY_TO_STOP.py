#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import time
import MOUNT_ARRAY_DEGRADED_BASIC

ARRAYNAME = MOUNT_ARRAY_DEGRADED_BASIC.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    if situ == "FAULT":
        return "pass"
    return "fail"

def set_result():
    result = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n")

def execute():
    out = MOUNT_ARRAY_DEGRADED_BASIC.execute()
    time.sleep(1)
    pos_util.pci_detach(MOUNT_ARRAY_DEGRADED_BASIC.ANY_ANOTHER_DATA)
    time.sleep(30)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()