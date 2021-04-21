#!/usr/bin/env python3

import sys
import os

import TEST_LIB
import TEST_SETUP_POS

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()

    for i in range(0, 5):
        TEST_SETUP_POS.unmount_array()
        TEST_SETUP_POS.mount_array()

    TEST_LIB.tear_down(test_name=filename)
