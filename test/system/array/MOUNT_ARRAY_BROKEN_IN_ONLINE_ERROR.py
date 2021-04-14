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
    if json_parser.is_online(detail) == False:
        return "pass"
    return "fail"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    if result == "pass":
        out = cli.get_pos_info()
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_ARRAY_NO_SPARE.execute()
    pos_util.pci_detach(MOUNT_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(0.1)
    pos_util.pci_detach(MOUNT_ARRAY_NO_SPARE.DATA_DEV_2)
    time.sleep(0.1)
    print ("try unmount")
    cli.unmount_array(MOUNT_ARRAY_NO_SPARE.ARRAYNAME)
    print ("unmount done")
    pos.exit_pos()
    print ("exit done")
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