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
import UPDATE_VOL_QOS_BASIC_1
import volume

ARRAYNAME = UPDATE_VOL_QOS_BASIC_1.ARRAYNAME
NAME = UPDATE_VOL_QOS_BASIC_1.NAME
SIZE = UPDATE_VOL_QOS_BASIC_1.SIZE
IOPS = 0
BW = (2**64-1) // 1024 // 1024    # Refer to SRS: http://globalwiki.itplatform.sec.samsung.net:8099/display/ibof/2.3.1+%5BIBOFOS_SW_FRID_0301%5D+Create+Volume


def execute():
    UPDATE_VOL_QOS_BASIC_1.execute()
    out = cli.update_volume_qos(NAME, str(IOPS), str(BW), ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)

