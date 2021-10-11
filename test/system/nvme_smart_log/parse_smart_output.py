#!/usr/bin/env python3
import json
import sys
import os


def parse_smart(arg, filePath):
    smartOutputFile = open(filePath, 'r')
    smartJson = json.load(smartOutputFile)
    if (arg == '1'):
        print(int(smartJson["temperature"]))
    elif (arg == '2'):
        print(int(smartJson["host_read_commands"]))
    elif (arg == '3'):
        print(int(smartJson["host_write_commands"]))
    elif (arg == '4'):
        print(int(smartJson["data_units_read"]))
    elif (arg == '5'):
        print(int(smartJson["data_units_written"]))


if __name__ == '__main__':
    parse_smart(sys.argv[1], sys.argv[2])
