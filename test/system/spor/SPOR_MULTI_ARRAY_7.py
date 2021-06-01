#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

arrays = [0]
volumes = [1, 2, 3]
current_test = 0
offset = 4096
size = '128k'

############################################################################
# Test Description
# write pattern A to several array with multiple volumes,
# create and delete array, and write different pattern to existing arrays,
# and simulate SPOR and verify each volume with latest pattern
############################################################################


def write_pattern():
    for arrayId in arrays:
        for volumeId in volumes:
            TEST_LIB.create_new_pattern(arrayId, volumeId)
            TEST_FIO.write(arrayId, volumeId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volumeId))


def test(arrayId_to_create="", arrayId_to_delete=""):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[Test {} Started]".format(current_test))

    write_pattern()

    if arrayId_to_create is not "":
        TEST_SETUP_POS.add_array(arrayId_to_create)
        for volumeId in volumes:
            TEST_SETUP_POS.create_subsystem(arrayId_to_create, volumeId)
            TEST_SETUP_POS.create_volume(arrayId_to_create, volumeId)
        arrays.append(arrayId_to_create)

    write_pattern()

    if arrayId_to_delete is not "":
        TEST_SETUP_POS.unmount_array(arrayId_to_delete)
        TEST_SETUP_POS.delete_array(arrayId_to_delete)
        arrays.remove(arrayId_to_delete)

    write_pattern()

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup(arrays)

    for arrayId in arrays:
        for volumeId in volumes:
            TEST_SETUP_POS.create_subsystem(arrayId, volumeId)
            TEST_SETUP_POS.mount_volume(arrayId, volumeId)

    for arrayId in arrays:
        for volumeId in volumes:
            TEST_FIO.verify(arrayId, volumeId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volumeId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def execute():
    test(arrayId_to_create=1, arrayId_to_delete=arrays[0])
    test(arrayId_to_create=0, arrayId_to_delete=arrays[0])
    test()


if __name__ == "__main__":
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup(arrays)
    for arrayId in arrays:
        for volumeId in volumes:
            TEST_SETUP_POS.create_subsystem(arrayId, volumeId)
            TEST_SETUP_POS.create_volume(arrayId, volumeId)

    execute()

    TEST_LIB.tear_down(filename, len(arrays))
