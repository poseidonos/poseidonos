#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../device/")

import SCAN_DEV_BASIC
import json_parser
import pos
import cli
import test_result

def set_result(detail):
    print ("set result")
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    print (result)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    SCAN_DEV_BASIC.execute()
    cli.mbr_reset()
    out = cli.create_array("wrong_uram", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "unvme-ns-3", "POSArray", "RAID5")
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()