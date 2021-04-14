#!/usr/bin/env python3

import json
import argparse
import sys
global volId
global ioType
global compare
global value
global group_report
parser = argparse.ArgumentParser(description='Please add fio option to fio_full_bench.    py')
parser.add_argument('-v','--volId', required=True, help='Pass this info for reading the volId to parse')
parser.add_argument('-io','--ioType', required=True, help='ioType read/write/mixed')
parser.add_argument('-c','--compare', required=False, help='BW value to compare')
parser.add_argument('-g','--group_report', required=True, help='Json logged as group report = 1 or 0')

args = parser.parse_args()
if(args.volId != None):
    volId = int(args.volId,0)
if(args.ioType != None):
    ioType = int(args.ioType,0)
if(args.compare != None):
    compare = int(args.compare, 0)
if(args.group_report != None):
    group_report = int(args.group_report, 0)
if(group_report == 1):
    job_pos=0
else:
    job_pos=volId
with open("fio_output.json") as jsonFile:
    data = json.load(jsonFile)
    jsonFile.close()
if(ioType == 0):
    value = data['jobs'][job_pos]['read']['bw']
    print(value)
if(ioType == 1):
    value = data['jobs'][job_pos]['write']['bw']
    print(value)

