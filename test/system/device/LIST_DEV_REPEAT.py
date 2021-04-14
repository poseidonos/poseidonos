#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
import json_parser
import pos
import cli
import test_result
import LIST_DEV_BASIC



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
    out1 = LIST_DEV_BASIC.execute()
    out2 = LIST_DEV_BASIC.execute()
    return out1, out2

if __name__ == "__main__":
    out1, out2 = execute()
    set_result(out1, out2)
    pos.kill_pos()