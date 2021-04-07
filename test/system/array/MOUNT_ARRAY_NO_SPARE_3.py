#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import ibofos
import cli
import test_result
import json
import CREATE_ARRAY_NO_SPARE

ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME
DATA_DEV_1 = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
DATA_DEV_2 = CREATE_ARRAY_NO_SPARE.DATA_DEV_2
DATA_DEV_3 = CREATE_ARRAY_NO_SPARE.DATA_DEV_3
REMAINING_DEV = CREATE_ARRAY_NO_SPARE.REMAINING_DEV
ANY_DATA = DATA_DEV_1


def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(out):
    data = json.loads(out)
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE":
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
    clear_result()
    CREATE_ARRAY_NO_SPARE.execute()
    out = cli.mount_ibofos()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()