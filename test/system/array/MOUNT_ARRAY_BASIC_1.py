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
import CREATE_ARRAY_BASIC_1

SPARE = CREATE_ARRAY_BASIC_1.SPARE
ANY_DATA = CREATE_ARRAY_BASIC_1.ANY_DATA
ANY_OTHER_DATA = CREATE_ARRAY_BASIC_1.ANY_OTHER_DATA

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def check_result(detail):
    data = json.loads(detail)
    state = data['Response']['info']['state']
    if state == "NORMAL":
        return "pass"
    return "fail"

def set_result(detail):
    result = check_result(detail)
    code = json_parser.get_response_code(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    CREATE_ARRAY_BASIC_1.execute()
    out = cli.mount_ibofos()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()