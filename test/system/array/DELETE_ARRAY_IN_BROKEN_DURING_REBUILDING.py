#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../lib/")
sys.path.append("../volume/")

import json_parser
import pos
import pos_util
import cli
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
    pos_util.pci_rescan()
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    pos_util.pci_detach(MOUNT_VOL_BASIC_1.ANY_DATA)
    pos_util.pci_detach("unvme-ns-1")
    time.sleep(1)
    out = cli.delete_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()
