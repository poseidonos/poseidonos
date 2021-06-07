#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import CREATE_VOL_BASIC_1
import volume

ARRAYNAME = CREATE_VOL_BASIC_1.ARRAYNAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    CREATE_VOL_BASIC_1.execute()
    wrong_vol = "wrong_vol"
    out = cli.rename_volume(wrong_vol, "new_vol", ARRAYNAME)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    pos.kill_pos()