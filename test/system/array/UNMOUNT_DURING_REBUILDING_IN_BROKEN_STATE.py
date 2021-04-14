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
import MOUNT_ARRAY_BASIC

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    data = json.loads(detail)
    state = data['Response']['info']['situation']
    if state == "OFFLINE":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        result = check_result(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    MOUNT_ARRAY_BASIC.execute()
    pos_util.pci_detach(MOUNT_ARRAY_BASIC.ANY_DATA)
    time.sleep(0.5)
    print(cli.list_device())
    pos_util.pci_detach(MOUNT_ARRAY_BASIC.ANY_OTHER_DATA)

    out = cli.unmount_array(MOUNT_ARRAY_BASIC.ARRAYNAME)
    print (out)

    cur_info = json.loads(cli.array_info(""))
    cur_state = cur_info['Response']['info']['situation']
    print(cur_state)
    if cur_state != 'FAULT':
        print("STOP State is not triggered, try again")
        return cur_info
    wait_time = 20
    for i in reversed(range(wait_time)):
        print ("Wait to cancel rebuild " + str(i) + " seconds left")
        time.sleep(1)

    out = cli.unmount_array(MOUNT_ARRAY_BASIC.ARRAYNAME)
    print (out)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()