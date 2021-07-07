#!/usr/bin/env python3

import sys
import os

import TEST_LIB
import TEST_SETUP_POS
import TEST_LOG

volId = 1
arrayId = 0
volId = 1


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()

    for i in range(0, 5):
        TEST_LOG.print_notice("Iteration {}".format(i))
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)

        TEST_SETUP_POS.unmount_volume(arrayId, volId)
        TEST_SETUP_POS.unmount_array(arrayId)

        TEST_SETUP_POS.mount_array(arrayId)
        TEST_SETUP_POS.delete_volume(arrayId, volId)
        TEST_SETUP_POS.unmount_array(arrayId)

        TEST_SETUP_POS.mount_array(arrayId)

    TEST_LIB.tear_down(test_name=filename)
