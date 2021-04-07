#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import ibofos
import cli
import test_result
import CREATE_ARRAY_BASIC_1

DATA = "unvme-ns-0,unvme-ns-1,unvme-ns-2"
ANY_DATA = "unvme-ns-0"
ANY_OTHER_DATA = "unvme-ns-1"
SPARE = "unvme-ns-3"

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    out = CREATE_ARRAY_BASIC_1.execute()
    cli.load_array("")
    cli.create_array("uram0", DATA, SPARE, "", "")
    cli.load_array("")
    out = cli.mount_ibofos()

    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()