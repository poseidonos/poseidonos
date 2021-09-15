#!/usr/bin/env python3
import subprocess
import os
import sys
import time

sys.path.append("../")
sys.path.append("../volume/")
sys.path.append("../../system/lib/")

import api
import cli
import pos
import json_parser
import test_result
import fio
import MOUNT_VOL_BASIC_1

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME


def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)


def execute():
    api.rescan_ssd()
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    api.detach_ssd(MOUNT_VOL_BASIC_1.ANY_DATA)
    api.detach_ssd("unvme-ns-1")
    time.sleep(1)
    out = cli.delete_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.flush_and_kill_pos()
