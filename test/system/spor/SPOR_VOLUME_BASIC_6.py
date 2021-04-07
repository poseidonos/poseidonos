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
patterns = ['\\\"ABAB\\\"', '\\\"CDCD\\\"', '\\\"efef\\\"', '\\\"ghgh\\\"']
volumes = [1, 2, 3, 4]

############################################################################
## Test Description
##  write data to several volumes,
##  delete a volume, and create a new volume with same id, and one more volume,
##  simulate SPOR and verify each volume with latest pattern
############################################################################
def test(volume_to_delete):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    for volId in volumes:
        TEST_FIO.write(volId, offset, size, volId)

    TEST_SETUP_IBOFOS.unmount_volume(volume_to_delete)
    TEST_SETUP_IBOFOS.delete_volume(volume_to_delete)

    TEST_SETUP_IBOFOS.create_subsystem(volume_to_delete)
    TEST_SETUP_IBOFOS.create_volume(volume_to_delete)

    for volId in volumes:
        TEST_FIO.write(volId, offset, size, volId + 1)

    TEST_SETUP_IBOFOS.trigger_spor()
    TEST_SETUP_IBOFOS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_IBOFOS.create_subsystem(volId)
        TEST_SETUP_IBOFOS.mount_volume(volId)

    for volId in volumes:
        TEST_FIO.verify(volId, offset, size, volId + 1)

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

def execute():
    for volId in volumes:
        test(volId)

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