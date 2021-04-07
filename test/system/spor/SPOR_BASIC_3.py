# !/usr/bin/env python3

import sys
import os
from itertools import product
from threading import Thread

import TEST
import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_IBOFOS

volumes = [1, 2]
current_test = 0

############################################################################
## Test Description
##  write pattern to two volumes, simulate SPOR, verify the pattern per each volume
############################################################################
def test(offset, size, testIdx):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    thread_list = []
    for volId in volumes:
        th = Thread(target=TEST_FIO.write, args=(volId, offset, size, testIdx + volId))
        thread_list.append(th)
        th.start()
    for th in thread_list:
        th.join()

    TEST_SETUP_IBOFOS.trigger_spor()
    TEST_SETUP_IBOFOS.dirty_bringup()
    for volId in volumes:
        TEST_SETUP_IBOFOS.create_subsystem(volId)
        TEST_SETUP_IBOFOS.mount_volume(volId)

    for volId in volumes:
        TEST_FIO.verify(volId, offset, size, testIdx + volId)

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

def execute():
    offsets = [0, 4096]
    sizes = ['128k', '256k']

    test(offset=0, size='1m', testIdx=1)
    if TEST.quick_mode == False:
        for i, (_offset, _size) in enumerate(product(offsets, sizes)):
            test(offset=_offset, size=_size, testIdx=i+1)

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_IBOFOS.clean_bringup()
    for volId in volumes:
        TEST_SETUP_IBOFOS.create_subsystem(volId)
        TEST_SETUP_IBOFOS.create_volume(volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)