#!/usr/bin/env python3

import psutil
import os
import sys
import datetime
#subprcess.call ('ls -al', shell=True)


def find_ibofos_coredump_and_renaming():
    core_dir_path = "/etc/pos/core"
    file_list = os.listdir(core_dir_path)
    for path in file_list:
        filename = os.path.basename(path)
        test_dir_from_filename = os.path.dirname(
            os.path.realpath(filename.replace('!', '/') + '/..'))
        test_path_from_script = os.path.dirname(
            os.path.realpath(__file__ + '/../..'))
        print("test dir from core file name : " + test_dir_from_filename +
              " test dir in this script : " + test_path_from_script)
        if("!poseidonos.core" in filename):
            if(filename != "poseidonos.core"):
                command = "mv " + core_dir_path + "/" + path\
                    + " " + core_dir_path + "/" + "poseidonos.core"
                print(command)
                os.system(command)
                print("\t#### Core Naming is Changed #####")
            else:
                print("\t#### Naming Change is already done ###")

    mtime = os.path.getmtime(core_dir_path + "/" + "poseidonos.core")
    date_core = datetime.datetime.fromtimestamp(mtime)
    date_now = datetime.datetime.now()
    date_diff = date_now - date_core
    if (date_diff.days >= 1):
        print("\t \033[31m WARNING Core file is modified long time ago,\n\
            Is this the core file you have tested? : %s \033[0m" % date_core)


def kill_wait(flag_kill="nokill"):
    for proc in psutil.process_iter():
        try:
            if "ibofos" in proc.name()[-6:] or "poseidonos" in proc.name()[-11:]:
                proc.resume()
                print("\t### process name : %s ####" % proc.name())
                if (flag_kill == "kill"):
                    print("\t### Process Kill ####")
                    proc.send_signal(11)
                print("\t### Process Wait ####")
                proc.wait()

        except psutil.NoSuchProcess:
            pass

    print("\t#### kill_and_wait.py Script Completely #####")


def check_free_space():
    temp_file_str = "df.temp"
    os.system("df > " + temp_file_str)
    f = open(temp_file_str)
    line_count = 0
    col_dir = 5
    col_size = 3
    available_size = 0
    for line in f:
        params = line.split()
        if (line_count == 0):
            col_index = 0
            for param in params:
                if ("Mounted" in param):
                    col_dir = col_index
                if ("Available" in param):
                    col_size = col_index
                col_index = col_index + 1
        else:
            if (params[col_dir] == "/"):
                available_size = int(params[col_size])
        line_count = line_count + 1
    f.close()
    os.system("rm " + temp_file_str)
    if (available_size < 40 * 1024 * 1024):
        print("\t \033[31m WARNING Disk Free Size is not so enough : boot disk size : %d \033[0m" % (available_size))
    else:
        print("free disk size : %d" % available_size)
