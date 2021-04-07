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


def check_result(out):
    data = json.loads(out)
    description = data['Response']['result']['status']['description']
    if "no any" in description:
        return "pass"
    return "fail"

def set_result(out):
    code = json_parser.get_response_code(out)
    result = check_result(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    ibofos.start_ibofos()
    out = cli.list_device()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()