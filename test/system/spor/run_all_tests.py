#!/usr/bin/env python3

import os
import sys
import glob
import subprocess

import TEST_LIB
import TEST_SETUP_POS

GREEN = "\033[1;32m"
RED = "\033[1;31m"
RESET = "\033[0;0m"

def print_msg(msg):
    print(GREEN + msg + RESET)


def print_err(msg):
    print(RED + msg + RESET)


def get_tests(filter):
    test_list = glob.glob(filter)
    test_list.sort()
    return test_list


def get_test_list(test_set_filter=[]):
    if(len(test_set_filter) == 0):
        test_set_filter = ['SPOR_*.py']
    else:
        test_set_filter = test_set_filter.split(", ")

    tests = []
    for test_name in test_set_filter:
        tests += get_tests(test_name)
    return tests


def run_test(testlist):
    print_msg("[Test list]\n- " + '\n- '.join(testlist))
    test_passed = list()
    test_failed = list()

    for test in testlist:
        ret = subprocess.call("python3 " + test + ' ' + ' '.join([x for x in sys.argv[1:]]), shell=True)
        if ret != 0:
            test_failed.append(test)
            break
        else:
            test_passed.append(test)

    test_passed.sort()
    test_failed.sort()

    test_result = [test_passed, test_failed]
    return test_result

if __name__ == "__main__":
    print_msg("Start SPOR Test")
    
    origin_value = TEST_LIB.is_journal_enabled().rstrip('\n')
    TEST_LIB.set_journal("true")

    if len(sys.argv) > 2 and sys.argv[-2] == "-s":
        tests = get_test_list(sys.argv[-1])
        sys.argv = sys.argv[:-2]
    else:
        tests = get_test_list()

    if len(sys.argv) > 6 and sys.argv[5] == "-r":
        run_time = sys.argv[6]
    else:
        run_time = 0

    result = run_test(tests)

    TEST_LIB.set_journal(origin_value)

    print_msg("SPOR Test Completed")
    print_msg("Passed test : ")
    print_msg('\n'.join([x for x in result[0]]))

    if len(result[1]) > 0:
        print_err("Failed test : ")
        print_err('\n'.join([x for x in result[1]]))
        sys.exit(1)