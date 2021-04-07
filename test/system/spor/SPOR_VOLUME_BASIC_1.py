#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_IBOFOS

current_test = 0
offset = 4096
size = '128k'

############################################################################
## Test Description
##  write pattern A to a volume, create one more volume,
##  write pattern to both volume,
##  and simulate SPOR and verify each volume with latest pattern
############################################################################
def execute():
    global current_test
    current_test = current_test + 1

    TEST_LOG.print_notice("[Test {} Started]".format(current_test))
    TEST_FIO.write(1, offset, size, 0)

    TEST_SETUP_IBOFOS.create_subsystem(volumeId=2)
    TEST_SETUP_IBOFOS.create_volume(volumeId=2)

    for volId in [1, 2]:
        TEST_FIO.write(volId, offset, size, 1)

    TEST_SETUP_IBOFOS.trigger_spor()
    TEST_SETUP_IBOFOS.dirty_bringup()

    for volId in [1, 2]:
        TEST_SETUP_IBOFOS.create_subsystem(volId)
        TEST_SETUP_IBOFOS.mount_volume(volId)

    for volId in [1, 2]:
        TEST_FIO.verify(volId, offset, size, 1)

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

if __name__ == "__main__":
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_IBOFOS.clean_bringup()
    TEST_SETUP_IBOFOS.create_subsystem(volumeId=1)
    TEST_SETUP_IBOFOS.create_volume(volumeId=1)

    execute()

    TEST_LIB.tear_down(test_name=filename)