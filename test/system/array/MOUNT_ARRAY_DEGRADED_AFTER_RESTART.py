#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import time
import MOUNT_ARRAY_NO_SPARE

def check_result(detail):
    state = json_parser.get_state(detail)
    if state == "DEGRADED":
        return "pass"
    return "fail"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        out = cli.get_pos_info()
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_ARRAY_NO_SPARE.execute()
    pos_util.pci_detach(MOUNT_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(0.1)
    cli.unmount_array(MOUNT_ARRAY_NO_SPARE.ARRAYNAME)
    pos.exit_pos()
    time.sleep(5)
    pos.start_pos()
    cli.scan_device()
    out = cli.mount_array(MOUNT_ARRAY_NO_SPARE.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()