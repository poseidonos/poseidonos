#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import json
import CREATE_ARRAY_BASIC

SPARE = CREATE_ARRAY_BASIC.SPARE
ANY_DATA = CREATE_ARRAY_BASIC.ANY_DATA
ANY_OTHER_DATA = CREATE_ARRAY_BASIC.ANY_OTHER_DATA
ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    CREATE_ARRAY_BASIC.execute()
    out = cli.mount_array(ARRAYNAME)
    print (out)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()