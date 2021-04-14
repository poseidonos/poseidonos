#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../start/")
import json_parser
import pos
import cli
import test_result
import START_POS_BASIC



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
    START_POS_BASIC.execute()
    out = cli.list_device()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()