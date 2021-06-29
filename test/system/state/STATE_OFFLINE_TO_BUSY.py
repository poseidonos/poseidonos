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
import time
import MOUNT_ARRAY_DEGRADED_BASIC


def check_result():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    if situ == "DEGRADED":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        result = check_result()
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    out = MOUNT_ARRAY_DEGRADED_BASIC.execute()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()