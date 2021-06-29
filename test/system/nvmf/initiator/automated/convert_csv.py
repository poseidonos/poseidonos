#!/usr/bin/env python3
import csv
import argparse
import subprocess
import os
#############################################################
sw_128k_limit = 9000
sr_128k_limit = 12300
rw_4k_limit = 6900
rr_4k_limit = 10000

sw_128k_test_name = "write_128KiB_QD4_BW"
sr_128k_test_name = "read_128KiB_QD4_BW"
rw_4k_test_name = "randwrite_4096B_QD128_BW"
rr_4k_test_name = "randread_4096B_QD128_BW"

ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../../../"
cur_dir = ibof_root + "/test/system/nvmf/initiator/automated"

path_128k= cur_dir + "/128k_log/full_result"
path_4k= cur_dir + "/4k_log/full_result"

test_name = "POS"
##############################################################

def read_test_result(filename):
    global result
    result = []
    if os.path.exists(filename) == False:
        return False
    r = open(filename, mode='rt', encoding='utf-8')
    while True: 
        line = r.readline()
        if not line:
            break
        result_line = line.split()
        result.append(result_line)
    r.close()
    return True

def get_revision():
    p1 = subprocess.Popen(["git", "log"], stdout = subprocess.PIPE)
    p2 = subprocess.Popen(["head", "-n", "1"], stdin=p1.stdout, stdout = subprocess.PIPE)
    p1.stdout.close()
    output = p2.communicate()[0].decode('utf-8')
    rev = output[7:17]
    return rev

def init_test_list():
    global test_name_list
    global test_value_list
    global max_perf_test_name_list 
    global max_perf_test_value_list

    test_name_list = ["Project", "Revision"]
    test_value_list = [args.test_name]  
    test_value_list.append(args.revision)
    max_perf_test_name_list = []
    max_perf_test_value_list = []
    

def add_max_perf_result(test_name, value):
    max_perf_test_name_list.append(test_name)
    max_perf_test_value_list.append(value)

def convert_test_result():

    for i in range(2, len(result)):
        for j in range(1, len(result[i])):
            if len(result[0]) != len(result[i]):
                print("Result does not exists")
                return False
            if j % 2 == 1:
                test_name = result[0][j] + "_" + result[0][j+1] + "_" + result[i][0].replace("=","") + "_" + result[1][j]                
            else:
                test_name = result[0][j-1] + "_" + result[0][j] + "_" +  result[i][0].replace("=","") + "_" + result[1][j]        
            if sw_128k_test_name == test_name or sr_128k_test_name == test_name:
                name = "Default_" + test_name
                test_name_list.insert(2, name)
                test_value_list.insert(2, result[i][j])
                add_max_perf_result(name, result[i][j])
            elif rw_4k_test_name == test_name or rr_4k_test_name == test_name:
                name = "Default_" + test_name
                test_name_list.insert(2, name)
                test_value_list.insert(2, result[i][j])
                add_max_perf_result(name, result[i][j])
                continue

            test_name_list.append(test_name)
            test_value_list.append(result[i][j])
            
    return True

def write_csv_file():
    if not args.revision:
        args.revision = "0"
    output = "AP_POS" + "_" + args.revision + ".csv"
    f = open(output, 'w', encoding='utf-8', newline='')
    wr = csv.writer(f)
    wr.writerow(test_name_list)
    wr.writerow(test_value_list)
    f.close()
    return True

def compare_and_print(idx, limit, actual_result): 
    success = True
    RED = '\033[91m'
    END = '\033[0m'
    GREEN  = '\33[32m'
    if float(actual_result) < limit:
        print(RED + max_perf_test_name_list[idx]," Below TH(",limit, "> ): ", actual_result + END)
        success &= False
    else:
        print(GREEN + max_perf_test_name_list[idx]," Above TH(", limit, "<= ): ", actual_result + END)
    return success

def check_performance_result():
    success = True
    global sw_128k_limit, sr_128k_limit, rw_4k_limit, rr_4k_limit
    if args.seq_write:
        sw_128k_limit = float(args.seq_write)
    if args.seq_read:
        sr_128k_limit = float(args.seq_read)
    if args.rand_write:
        rw_4k_limit = float(args.rand_write)
    if args.rand_read:
        rr_4k_limit = float(args.rand_read)

    for i in range(len(max_perf_test_name_list)):
        if sw_128k_test_name in max_perf_test_name_list[i]:
            success &= compare_and_print(i, sw_128k_limit, max_perf_test_value_list[i])
                
        elif sr_128k_test_name in max_perf_test_name_list[i]:
            success &= compare_and_print(i, sr_128k_limit, max_perf_test_value_list[i])

        elif rw_4k_test_name in max_perf_test_name_list[i]:
            success &= compare_and_print(i, rw_4k_limit, max_perf_test_value_list[i])

        elif rr_4k_test_name in max_perf_test_name_list[i]:
            success &= compare_and_print(i, rr_4k_limit, max_perf_test_value_list[i])
    return success

def parse_arguments():
    parser = argparse.ArgumentParser(description='Convert Result')
    parser.add_argument('-f', '--path_128k', default=path_128k,\
            help='Select 128k Test Result to Convert to csv, Default: ' + path_128k)
    parser.add_argument('-p', '--path_4k', default=path_4k,\
            help='Select 4k Test Result to convert to csv, Default: ' + path_4k)
    parser.add_argument('-r', '--revision', default=0, \
            help='Set Revision, Default: 0')
    parser.add_argument('-t', '--test_name', default=test_name, \
            help='Set Test Name, Default:' + test_name)
    parser.add_argument('-sw', '--seq_write', default=sw_128k_limit,\
            help='Set 128k seq. write limit, Default: ' + str(sw_128k_limit) + ' MB/s')
    parser.add_argument('-sr', '--seq_read', default=sr_128k_limit,\
            help='Set 128k seq. read limit, Default: ' + str(sr_128k_limit) + ' MB/s')
    parser.add_argument('-rw', '--rand_write', default=rw_4k_limit,\
            help='Set 4k random write limit, Default: ' + str(rw_4k_limit) + ' MB/s')
    parser.add_argument('-rr', '--rand_read', default=rr_4k_limit, \
            help='Set 4k random read limit, Default: ' + str(rr_4k_limit) + ' MB/s')
    global args
    args = parser.parse_args()

if __name__=="__main__":
    parse_arguments()
    init_test_list()
    success = read_test_result(args.path_128k)
    success &= convert_test_result()
    success &= read_test_result(args.path_4k)
    success &=convert_test_result()
    success &=write_csv_file()
    success &=check_performance_result()
    if success:
        exit(0)
    else:
        exit(-1)
