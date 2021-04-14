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
import CREATE_ARRAY_NO_SPARE

DATA = "unvme-ns-0,unvme-ns-1,unvme-ns-2"
SPARE = "unvme-ns-3"
ARRAY_NAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME

def check_result(detail):
    state = json_parser.is_online(detail)
    if state == False:
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
    pos_util.pci_rescan()
    out = CREATE_ARRAY_NO_SPARE.execute()
    print (out)
    out = cli.mount_array(ARRAY_NAME)
    print (out)
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(2)
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_2)
    time.sleep(2)
    out = cli.unmount_array(ARRAY_NAME)
    print (out)
    out = cli.delete_array(ARRAY_NAME)
    print (out)
    pos.exit_pos()
    pos_util.pci_rescan()
    pos.start_pos()
    cli.scan_device()
    cli.list_device()
    out = cli.create_array("uram0", DATA, SPARE, ARRAY_NAME, "RAID5")
    print (out)
    out = cli.mount_array(ARRAY_NAME)
    print (out)

    return out
if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()