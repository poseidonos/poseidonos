#!/usr/bin/env python3

import sys
import os

from threading import Thread

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

log_buffer_size_mb = 50
current_test = 0
offset = 4096
size = '128k'
arrayId = 0
volumes = [1, 2, 3, 4]

############################################################################
# Test Description
# Write data to several volumes and delete one of the volumes at the same time
# to test volume deletion while writing data to other volumes.
# Then simulate SPOR and verify each volume except the deleted one.
# Test with small-sized log buffer to trigger checkpoint in the meantime.
############################################################################


def test(volume_to_delete):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    thread_list = []
    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        th = Thread(target=TEST_FIO.write, args=(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId)))
        thread_list.append(th)

    for th in thread_list:
        th.start()

    TEST_SETUP_POS.unmount_volume(arrayId, volume_to_delete)
    TEST_SETUP_POS.delete_volume(arrayId, volume_to_delete)

    volumes.remove(volume_to_delete)

    for th in thread_list:
        th.join()

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def execute():
    for volId in volumes:
        test(volId)


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)
    TEST_LIB.set_log_buffer_size(log_buffer_size_mb)

    TEST_SETUP_POS.clean_bringup()
    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)
    TEST_LIB.set_log_buffer_size(0)
