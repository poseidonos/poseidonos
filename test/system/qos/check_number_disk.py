#!/usr/bin/env python3

import os
import json
import sys
import getopt

file_path = os.path.dirname(os.path.abspath(__file__))
device_list_file = str(file_path) + "/dev_list.log"


def count_number():
    devFile = open(str(device_list_file), "r")
    data = devFile.read()
    occurences = data.count("unvme-ns")
    print(occurences)
    return occurences


def main():
    ret = count_number()
    sys.exit(ret)


if __name__ == '__main__':
    main()
