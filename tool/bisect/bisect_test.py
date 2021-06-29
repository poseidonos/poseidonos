#!/usr/bin/env python3

import argparse
import subprocess
import os

default_good_commit='pos-0.5.2'
default_bad_commit='HEAD'

def parse_arguments():
    parser = argparse.ArgumentParser(description='Find first bad commit')
    parser.add_argument('-b', '--bad', default=default_bad_commit, help='Set first bad commit or tag, default: ' + default_bad_commit)
    parser.add_argument('-g', '--good', default=default_good_commit, help='Set last good commit or tag, default: ' + default_good_commit)
    parser.add_argument('-p', '--path', required=True, help='Set test script path. Test should return 0 or 1 at the case of fail and success, respectively.')
    global args
    args = parser.parse_args()

def run_bisect():
    badcommit = args.bad
    goodcommit = args.good
    test_script = os.path.abspath(args.path)
    cwd = os.path.dirname(os.path.abspath(__file__))
    ibof_root = cwd + "/../../"
    os.chdir(ibof_root)

    subprocess.call(["git", "bisect", "reset"])
    subprocess.call(["git", "bisect", "start", badcommit, goodcommit])
    subprocess.call(["git", "bisect", "run", test_script])
    return 1

if __name__ == "__main__":
    parse_arguments()
    result = run_bisect()
    exit(result)
