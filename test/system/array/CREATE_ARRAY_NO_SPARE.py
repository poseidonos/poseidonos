#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import ibofos
import cli
import test_result

ARRAYNAME = "POSArray"
DATA_DEV_1 = "unvme-ns-0"
DATA_DEV_2 = "unvme-ns-1"
DATA_DEV_3 = "unvme-ns-2"
REMAINING_DEV = "unvme-ns-3"

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
    ibofos.start_ibofos()
    cli.scan_device()
    out = cli.create_array("uram0", ",".join([DATA_DEV_1,DATA_DEV_2,DATA_DEV_3]), "", ARRAYNAME, "")
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()