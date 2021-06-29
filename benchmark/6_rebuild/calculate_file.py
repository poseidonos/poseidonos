#!/usr/bin/env python3
import os

directory = ["./rebuild_fio_result_init_1/", "./rebuild_fio_result_init_2/"]
outfile_name ="rebuild_rand_result.txt"
rand_out_file = open(outfile_name, 'w')

files = []
for d in directory:
    for dirpath, _, filenames in os.walk(d):
        for filename in filenames:
            files.append(dirpath + "/" + filename)
        
rand_write_bw = []
rand_read_bw = []
rand_write_iops = []
rand_read_iops = []

for filename in files:
    if ".log" not in filename:
        continue
    file = open(filename)
    write_temp = []
    read_temp = []
    for line in file:
        split = line.split(",")
        if "1" in split[2]:
            write_temp.append(split[1])
        else:
            read_temp.append(split[1])
    if "rand" in filename and "bw" in filename:
        rand_write_bw.append(write_temp)
        rand_read_bw.append(read_temp)
    elif "rand" in filename and "iops" in filename:
        rand_write_iops.append(write_temp)
        rand_read_iops.append(read_temp)
    file.close()

rand_out_file.write("rand_write_bw, rand_read_bw, rand_write_iops, rand_read_iops\n")
for j in range(len(rand_write_bw[1])):
    sum = 0
    for i in range(len(rand_write_bw)):
        if len(rand_write_bw[i]) <= j:
            continue
        sum += int(rand_write_bw[i][j])
    rand_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(rand_read_bw)):
        if len(rand_read_bw[i]) <= j:
            continue
        sum += int(rand_read_bw[i][j])
    rand_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(rand_write_iops)):
        if len(rand_write_iops[i]) <= j:
            continue
        sum += int(rand_write_iops[i][j])
    rand_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(rand_read_iops)):
        if len(rand_read_iops[i]) <= j:
            continue
        sum += int(rand_read_iops[i][j])
    rand_out_file.write(str(sum) + "\n")
rand_out_file.close()

