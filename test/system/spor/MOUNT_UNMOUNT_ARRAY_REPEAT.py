#!/usr/bin/env python3

import sys
import os

import TEST_LIB
import TEST_SETUP_IBOFOS

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_IBOFOS.clean_bringup()

    for i in range(0, 5):
        TEST_SETUP_IBOFOS.unmount_array()
        TEST_SETUP_IBOFOS.mount_array()

    TEST_LIB.tear_down(test_name=filename)
