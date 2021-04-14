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
import pos_constant
import START_POS_BASIC


def check_result(detail):
    isOnline = json_parser.is_online(detail)
    capacity = json_parser.get_capacity(detail)
    used =json_parser.get_used(detail)

    if isOnline == False and capacity == 0 and used == 0:
        return "pass"
    return "fail"

def set_result(detail):
    result = check_result(detail)
    code = json_parser.get_response_code(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    START_POS_BASIC.execute()
    out = cli.get_pos_info()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()