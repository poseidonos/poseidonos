#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../fault-tolerance/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import time
import REBUILD_STOP_BASIC

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n" + detail)

def execute():
    REBUILD_STOP_BASIC.execute()
    out = cli.add_device(REBUILD_STOP_BASIC.REMAINING_DEV, REBUILD_STOP_BASIC.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()