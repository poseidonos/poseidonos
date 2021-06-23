#!/usr/bin/python3

import os
import sys
import time


default_result_file = "/tmp/fio.qos.result"

f = open(default_result_file)
total = 0
count = 0
for line in f:
    if not "bw=" in line:
        continue
    strings = line.split()
    parsed_str = ""
    for st in strings:
        if "bw=" in st:
            parsed_str = st.split("=")[1]
            break
    if ("M" in parsed_str):
        performance = float(parsed_str.split("M")[0])
    elif ("G" in parsed_str):
        performance = float(parsed_str.split("G")[0]) * 1024
    else:
        print("So low performance !!!!!!")
        assert(0)
    count = count + 1
    total = total + performance

print(int((float(total) / count)))
