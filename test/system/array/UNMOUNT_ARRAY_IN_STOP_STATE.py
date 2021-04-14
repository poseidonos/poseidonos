#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../state/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import STATE_BUSY_TO_STOP
import fio
import time

def check_result(detail):
    state = json_parser.is_online(detail)
    if state == False:
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    if result == "pass":
        result = check_result(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    STATE_BUSY_TO_STOP.execute()
    out = cli.unmount_array(STATE_BUSY_TO_STOP.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()