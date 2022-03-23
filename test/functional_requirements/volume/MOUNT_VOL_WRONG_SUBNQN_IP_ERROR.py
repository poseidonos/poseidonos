#!/usr/bin/env python3
import subprocess
import os
import sys

sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json
import json_parser
import pos
import cli
import api
import CREATE_VOL_BASIC_1

VOL_NAME = CREATE_VOL_BASIC_1.VOL_NAME
VOL_SIZE = CREATE_VOL_BASIC_1.VOL_SIZE
VOL_IOPS = CREATE_VOL_BASIC_1.VOL_IOPS
VOL_BW = CREATE_VOL_BASIC_1.VOL_BW

SPARE = CREATE_VOL_BASIC_1.SPARE
ANY_DATA = CREATE_VOL_BASIC_1.ANY_DATA

ARRAYNAME = CREATE_VOL_BASIC_1.ARRAYNAME



def execute():
    CREATE_VOL_BASIC_1.execute()
    out = cli.mount_volume(CREATE_VOL_BASIC_1.VOL_NAME, ARRAYNAME, "wrongsubnqn@#", "uknowntrtype", "unknokwntraddr", "unknowntrsvcid")
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
