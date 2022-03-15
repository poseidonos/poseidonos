#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import MOUNT_VOLS_ON_TWO_RAID0_ARRAYS
import fio
import time
ARRAY1NAME = MOUNT_VOLS_ON_TWO_RAID0_ARRAYS.ARRAY1NAME
ARRAY2NAME = MOUNT_VOLS_ON_TWO_RAID0_ARRAYS.ARRAY2NAME

def execute():
    MOUNT_VOLS_ON_TWO_RAID0_ARRAYS.execute()
    fio_proc1 = fio.start_fio(0, 30, ARRAY1NAME, "subsystem1")
    fio_proc2 = fio.start_fio(0, 30, ARRAY2NAME, "subsystem2")
    fio.wait_fio(fio_proc1)
    fio.wait_fio(fio_proc2)
    if api.wait_situation(ARRAY1NAME, "NORMAL", 0) == True:
        if api.wait_situation(ARRAY2NAME, "NORMAL", 0) == True:
            return "pass", cli.array_info(ARRAY2NAME)
        else:
            return "fail", cli.array_info(ARRAY2NAME)
    else:
        return "fail", cli.array_info(ARRAY1NAME)

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result, response = execute()
    ret = api.set_result_manually(response, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)