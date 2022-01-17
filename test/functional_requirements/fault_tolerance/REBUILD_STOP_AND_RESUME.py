#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
import json_parser
import pos
import pos_util
import cli
import api
import json
import MOUNT_VOL_NO_SPARE
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_NO_SPARE.ANY_DATA
FIRST_SPARE = "unvme-ns-4"
SECOND_SPARE = MOUNT_VOL_NO_SPARE.REMAINING_DEV
ARRAYNAME = MOUNT_VOL_NO_SPARE.ARRAYNAME


def execute():
    MOUNT_VOL_NO_SPARE.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    api.detach_ssd_and_attach(DETACH_TARGET_DEV)
    code = -1
    retry = 0
    maxRetry = 20
    while code != 0:
        retry = retry + 1
        if retry is maxRetry:
            return "fail"
        out = cli.add_device(FIRST_SPARE, ARRAYNAME)
        code = json_parser.get_response_code(out)
        print("add spare, try: " + str(retry) + "response: " + str(code))
    print(FIRST_SPARE + " is added as a spare")
    out = cli.add_device(SECOND_SPARE, ARRAYNAME)
    code = json_parser.get_response_code(out)
    if code == 0:
        print(SECOND_SPARE + " is added as a spare")

    if api.wait_situation(ARRAYNAME, "REBUILDING") is True:
        print("1st rebuilding")
        api.detach_ssd(FIRST_SPARE)
        print(FIRST_SPARE + " detachment is triggered")
        if api.is_device_exists(FIRST_SPARE) is False:
            print(FIRST_SPARE + " is detached")
        timeout = 80000  # 80s
        elapsed_ms = 0
        while api.is_spare_device(ARRAYNAME, SECOND_SPARE) is True:
            print("Wait for 2nd rebuilding")
            time.sleep(0.01)
            elapsed_ms += 10
            if elapsed_ms > timeout:
                return "fail"
        print("2nd rebuilding started")
        maxRebuildTimeout = 300 * 1000  # 300s
        if api.wait_situation(ARRAYNAME, "NORMAL", maxRebuildTimeout) is True:
            return "pass"
    return "fail"


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
