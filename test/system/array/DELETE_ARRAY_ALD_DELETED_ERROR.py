#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import DELETE_ARRAY_AFTER_UNMOUNT

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    out = DELETE_ARRAY_AFTER_UNMOUNT.execute()
    ret = json_parser.get_response_code(out)
    if ret == 0:
        out = cli.delete_array(DELETE_ARRAY_AFTER_UNMOUNT.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()