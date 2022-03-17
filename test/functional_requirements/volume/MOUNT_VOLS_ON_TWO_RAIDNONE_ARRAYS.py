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
import test_result
import CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS

VOL1_NAME = CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS.VOL1_NAME
VOL2_NAME = CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS.VOL2_NAME
ARRAY1NAME = CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS.ARRAY1NAME
ARRAY2NAME = CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS.ARRAY2NAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")


def execute():
    clear_result()
    CREATE_VOLS_ON_TWO_RAIDNONE_ARRAYS.execute()
    out1 = cli.mount_volume(VOL1_NAME, ARRAY1NAME, "")
    print(out1)
    code = json_parser.get_response_code(out1)
    if code is 0:
        out2 = cli.mount_volume(VOL2_NAME, ARRAY2NAME, "")
        print(out2)
        return out2
    else:
        return out1


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)