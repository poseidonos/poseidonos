#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import test_result
import json
import UNMOUNT_ARRAY_WITH_VOL_MOUNTED

ARRAYNAME = UNMOUNT_ARRAY_WITH_VOL_MOUNTED.ARRAYNAME


def check_result(detail):
    state = json_parser.is_online(detail)
    if state == False:
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = check_result(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    UNMOUNT_ARRAY_WITH_VOL_MOUNTED.execute()
    out = pos.exit_pos()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()