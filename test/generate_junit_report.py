#!/bin/python

import sys
import os
import commands

# ctest does not pass in gtest option "--gtest_output" properly. Hence, we provide a wrapper instead, to run gtest directly.
# this is for CI integration. Developers (i.e., non CI) are still expected to run "make ut" to run tests through ctest and check the test summary.

BUILD_DIR = "build"
CMD_TMPL  = "{0}/{1} --gtest_output=xml:build/{1}-test.xml"

def run_test(test_suffix):
    test_files = os.listdir(BUILD_DIR)
    processed = 0
    for test_file in test_files:
        if not test_file.endswith( test_suffix ):
            continue
        cmd = CMD_TMPL.format(BUILD_DIR, test_file)
        print commands.getoutput(cmd)
        processed += 1
    print "Processed/generated %d junit xmls at %s/" % (processed, BUILD_DIR)

def main():
    if len(sys.argv) != 2:
        print("Proper usage: generate_junit_report.py [ut|it|st]")
        sys.exit(1)

    test_suffix = sys.argv[1]
    allowed_types = ["ut", "it", "st"]
    if test_suffix not in allowed_types:
        print("Only the followings are accepted: " + allowed_types)
        sys.exit(1)
    
    run_test(test_suffix)

main()