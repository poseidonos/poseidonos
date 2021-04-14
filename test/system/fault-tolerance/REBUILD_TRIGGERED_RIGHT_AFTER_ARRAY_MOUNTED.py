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
import CREATE_ARRAY_BASIC


def check_result(detail):
    state = json_parser.get_state(detail)
    if state == "REBUILD":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        detail = cli.get_pos_info()
        result = check_result(detail)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    CREATE_ARRAY_BASIC.execute()
    pos_util.pci_detach(CREATE_ARRAY_BASIC.ANY_DATA)
    time.sleep(1)
    out = cli.mount_array(CREATE_ARRAY_BASIC.ARRAYNAME)
    time.sleep(5)
    print(out)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()