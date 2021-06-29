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
import ADD_SPARE_BASIC

SPARE_DEV = ADD_SPARE_BASIC.SPARE_DEV


def check_result(out):
    data = json.loads(out)
    list = []
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] == SPARE_DEV :
            return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        result = check_result(detail)
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    ADD_SPARE_BASIC.execute()
    cli.unmount_array(ADD_SPARE_BASIC.ARRAYNAME)
    pos.exit_pos()
    pos.start_pos()
    cli.scan_device()
    cli.mount_array(ADD_SPARE_BASIC.ARRAYNAME)
    out = cli.array_info(ADD_SPARE_BASIC.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()