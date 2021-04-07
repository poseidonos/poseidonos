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
import CREATE_ARRAY_BASIC_2

ARRAY_NAME = CREATE_ARRAY_BASIC_2.ARRAY_NAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def check_result(out1, out2):
    print(out1)
    print("")
    print(out2)
    list1 = json.loads(out1)['Response']['result']['data']['devicelist']
    list2 = json.loads(out2)['Response']['result']['data']['devicelist']

    if list1 == list2:
        return "pass"
    return "fail"

def set_result(out1, out2):
    result = check_result(out1, out2)
    code = json_parser.get_response_code(out2)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out1 + "\n" + out2)

def execute():
    clear_result()
    CREATE_ARRAY_BASIC_2.execute()
    list1 = cli.array_info(ARRAY_NAME)
    ibofos.exit_ibofos()
    ibofos.start_ibofos()
    cli.scan_device()
    out = cli.load_array(ARRAY_NAME)
    print(out)
    list2 = cli.array_info(ARRAY_NAME)
    return list1, list2

if __name__ == "__main__":
    out1, out2 = execute()
    set_result(out1, out2)
    ibofos.kill_ibofos()