#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import time
import test_result
import ibofos_constant
import CREATE_ARRAY_BASIC_1
import ibofos_util

IBOFOS_ROOT = '../../../'

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    CREATE_ARRAY_BASIC_1.execute()
    ibofos.exit_ibofos()
    
    ibofos_mbr_reset = IBOFOS_ROOT + "/test/script/mbr_reset.sh"
    subprocess.call([ibofos_mbr_reset])

    ibofos.start_ibofos()
    out = cli.load_array("")

    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()
