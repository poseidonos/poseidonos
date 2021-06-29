#!/usr/bin/env python3
import os

directory = ["./longterm_fio_result_init_1/", "./longterm_fio_result_init_2/"]
outfile_name =["seq_result.txt", "rand_result.txt"]
seq_out_file = open(outfile_name[0], 'w')
rand_out_file = open(outfile_name[1], 'w')

files = []
for d in directory:
    for dirpath, _, filenames in os.walk(d):
        for filename in filenames:
            files.append(dirpath + "/" + filename)
        
seq_write_bw = []
seq_read_bw = []
seq_write_iops = []
seq_read_iops = []
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
    if "seq" in filename and "bw" in filename:
        seq_write_bw.append(write_temp)
        seq_read_bw.append(read_temp)
    elif "seq" in filename and "iops" in filename:
        seq_write_iops.append(write_temp)
        seq_read_iops.append(read_temp)
    elif "rand" in filename and "bw" in filename:
        rand_write_bw.append(write_temp)
        rand_read_bw.append(read_temp)
    elif "rand" in filename and "iops" in filename:
        rand_write_iops.append(write_temp)
        rand_read_iops.append(read_temp)
    file.close()

seq_out_file.write("seq_write_bw, seq_read_bw, seq_write_iops, seq_read_iops\n")
rand_out_file.write("rand_write_bw, rand_read_bw, rand_write_iops, rand_read_iops\n")
#min_len=min(len(seq_write_bw[1]), len(rand_write_bw[1]), len(rand_read_bw[1]), len(rand_write_iops[1]))
#print(min_len)

seq_max = 0
for index in seq_write_bw:
    seq_max = max(seq_max, len(index))

for j in range(seq_max):
    sum = 0
    for i in range(len(seq_write_bw)):
        if len(seq_write_bw[i]) <= j:
            continue
        sum += int(seq_write_bw[i][j])
    seq_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(seq_read_bw)):
        if len(seq_read_bw[i]) <= j:
            continue
        sum += int(seq_read_bw[i][j])
    seq_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(seq_write_iops)):
        if len(seq_write_iops[i]) <= j:
            continue
        sum += int(seq_write_iops[i][j])
    seq_out_file.write(str(sum) + ",")
    sum = 0
    for i in range(len(seq_read_iops)):
        if len(seq_read_iops[i]) <= j:
            continue
        sum += int(seq_read_iops[i][j])
    seq_out_file.write(str(sum) + "\n")

rand_max = 0
for index in rand_write_bw:
    rand_max = max(rand_max, len(index))
    
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
seq_out_file.close()
rand_out_file.close()

