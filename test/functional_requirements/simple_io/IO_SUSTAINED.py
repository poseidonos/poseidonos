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
import MOUNT_ARRAY_BASIC
import fio
import time
import pos_constant

ARRAYNAME = "POSArray"
VOLNAME1 = "vol1"
VOLNAME2 = "vol2"
VOLSIZE1 = pos_constant.SIZE_1GB * 56
VOLSIZE2 = pos_constant.SIZE_1GB * 55

def execute():
    MOUNT_ARRAY_BASIC.execute()
    print (cli.create_volume(VOLNAME1, str(VOLSIZE1), "", "", ARRAYNAME))
    print (cli.create_volume(VOLNAME2, str(VOLSIZE2), "", "", ARRAYNAME))
    print (cli.mount_volume(VOLNAME1, ARRAYNAME, ""))
    print (cli.mount_volume(VOLNAME2, ARRAYNAME, ""))
    time_seq = 60 * 20
    time_mix = 3600 * 48
    #sequentialwrite
    print("sequential write begin")
    fio_proc1 = fio.start_fio(0, time_seq)
    fio_proc2 = fio.start_fio(1, time_seq)
    fio.wait_fio(fio_proc1)
    fio.wait_fio(fio_proc2)
    print("sequential write end")
    print("random write begin")
    fio_proc1 = fio.start_fio(0, time_mix, "randwrite")
    fio_proc2 = fio.start_fio(1, time_mix, "randwrite")
    fio.wait_fio(fio_proc1)
    fio.wait_fio(fio_proc2)
    print("random write end")


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    execute()