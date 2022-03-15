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
import pos_constant
import MOUNT_TWO_RAID0_ARRAYS
import volume

VOL1_NAME = "vol1"
VOL1_SIZE = pos_constant.SIZE_1GB
VOL1_IOPS = 0
VOL1_BW = 0

VOL2_NAME = "vol2"
VOL2_SIZE = pos_constant.SIZE_1GB
VOL2_IOPS = 0
VOL2_BW = 0

ARRAY1NAME = MOUNT_TWO_RAID0_ARRAYS.ARRAY1NAME
ARRAY2NAME = MOUNT_TWO_RAID0_ARRAYS.ARRAY2NAME


def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")


def execute():
    clear_result()
    MOUNT_TWO_RAID0_ARRAYS.execute()
    out1 = cli.create_volume(VOL1_NAME, str(VOL1_SIZE), "", "", ARRAY1NAME)
    print(out1)
    code = json_parser.get_response_code(out1)
    if code is 0:
        out2 = cli.create_volume(VOL2_NAME, str(VOL2_SIZE), "", "", ARRAY2NAME)
        print(out2)
        return out2
    else:
        return out1


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)