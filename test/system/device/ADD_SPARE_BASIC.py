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
import MOUNT_ARRAY_NO_SPARE

SPARE_DEV = MOUNT_ARRAY_NO_SPARE.REMAINING_DEV
ARRAYNAME = MOUNT_ARRAY_NO_SPARE.ARRAYNAME

def check_result(out):
    data = json.loads(out)
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] == SPARE_DEV :
            return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        out = cli.array_info(ARRAYNAME)
        result = check_result(out)
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_ARRAY_NO_SPARE.execute()
    out = cli.add_device(SPARE_DEV, ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()