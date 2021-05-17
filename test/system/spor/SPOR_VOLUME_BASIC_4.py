#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

current_test = 0
offset = 4096
size = '128k'
arrayId = 0
volumes = [1, 2, 3, 4]

############################################################################
# Test Description
# write data to several volumes,
# create or delete several volume, and write different pattern to every volumes,
# and simulate SPOR and verify each volume with latest pattern
############################################################################


def test_create(volume_to_create):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.create_subsystem(arrayId, volume_to_create)
    TEST_SETUP_POS.create_volume(arrayId, volume_to_create)
    volumes.append(volume_to_create)

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def test_delete(volume_to_delete):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.unmount_volume(arrayId, volume_to_delete)
    TEST_SETUP_POS.delete_volume(arrayId, volume_to_delete)
    volumes.remove(volume_to_delete)

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def execute():
    test_delete(volume_to_delete=2)
    test_create(volume_to_create=5)
    test_delete(volume_to_delete=5)
    test_delete(volume_to_delete=4)


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()
    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)
