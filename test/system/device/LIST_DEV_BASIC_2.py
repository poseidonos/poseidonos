#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
import json_parser
import ibofos
import cli
import test_result
import LIST_DEV_BASIC_1

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")


def check_result(out1, out2):
    list1 = json.loads(out1)['Response']['result']['data']['devicelist']
    list2 = json.loads(out2)['Response']['result']['data']['devicelist']

    if list1 == list2:
        return "pass"
    return "fail"

def set_result(out1, out2):
    code = json_parser.get_response_code(out2)
    result = check_result(out1, out2)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out1 + "\n" + out2)

def execute():
    clear_result()
    out1 = LIST_DEV_BASIC_1.execute()
    out2 = LIST_DEV_BASIC_1.execute()
    return out1, out2

if __name__ == "__main__":
    out1, out2 = execute()
    set_result(out1, out2)
    ibofos.kill_ibofos()