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

ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME
DATA_DEV_1 = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
DATA_DEV_2 = CREATE_ARRAY_NO_SPARE.DATA_DEV_2
ANY_DATA = DATA_DEV_1
REMAINING_DEV = CREATE_ARRAY_NO_SPARE.REMAINING_DEV

def check_result(out):
    data = json.loads(out)
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] != "":
            return "fail"
    return "pass"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        out = cli.array_info("")
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    CREATE_ARRAY_NO_SPARE.execute()
    out = cli.mount_array(ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
