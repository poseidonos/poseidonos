#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import test_result
import json
import ibofos_util
import MOUNT_ARRAY_NO_SPARE_3

ARRAYNAME = DATA_DEV_1 = MOUNT_ARRAY_NO_SPARE_3.ARRAYNAME
DATA_DEV_1 = MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_1
DATA_DEV_2 = MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_2
DATA_DEV_3 = MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_3
SPARE_DEV = MOUNT_ARRAY_NO_SPARE_3.REMAINING_DEV

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_NO_SPARE_3.execute()
    ibofos_util.pci_detach(MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_1)
    time.sleep(2)
    ibofos_util.pci_detach(MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_2)
    time.sleep(2)
    ibofos_util.pci_detach(MOUNT_ARRAY_NO_SPARE_3.DATA_DEV_3)
    time.sleep(2)
    cli.array_info(MOUNT_ARRAY_NO_SPARE_3.ARRAYNAME)
    out = cli.add_device(SPARE_DEV, MOUNT_ARRAY_NO_SPARE_3.ARRAYNAME)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()