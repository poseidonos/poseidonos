#!/usr/bin/env python3

import subprocess
import sys
import psutil

def start_pos(log_path, ibof_root):
    global ibof_proc
    print ("\tStarting POS .. (log path: " + log_path+ ")")
    ibof_execution = ibof_root + "bin/poseidonos"
    with open(log_path, "w") as output_file:
        ibof_proc = subprocess.Popen(ibof_execution, \
                stdout=output_file, stderr=output_file)
    subprocess.call(["sleep", "3"])
    print ("\tPOS Started!")

def bringup_pos(**args):
    args['volume_cnt'] = 1
    args['subsystem_cnt'] = 1
    bringup_multiple_volume(**args)

def bringup_multiple_volume(**args):
    start_pos(args['log_path'], args['ibof_root'])
    pos_bringup = args['ibof_root'] + \
            "/test/system/io_path/setup_ibofos_nvmf_volume.sh"
    print("\tBringup POS")
    if 'clean' not in args:
        args['clean'] = 1
    if 'volume_size' not in args:
        args['volume_size'] = 2147483648
     
    subprocess.call([pos_bringup, \
            "-t", args['transport'], \
            "-a", args['target_ip'], \
            "-v", str(args['volume_cnt']), \
            "-s", str(args['subsystem_cnt']), \
            "-S", str(args['volume_size']) + "B", \
            "-c", str(args['clean'])], \
            stdout=args['stdout_type'], stderr=subprocess.STDOUT)
    print("\tBringup Done")

def terminate_pos(ibof_root, stdout_type):
    print("\tTerminate POS")
    if 'ibof_proc' in globals():
        ibof_cli = ibof_root + "bin/poseidonos-cli"
        subprocess.call([ibof_cli, "array", "unmount", "--array-name", "POSArray", "--force"],\
            stdout=stdout_type, stderr=subprocess.STDOUT)
        subprocess.call([ibof_cli, "system", "stop", "--force"],\
            stdout=stdout_type, stderr=subprocess.STDOUT)
        ibof_proc.wait()
    print("\tTerminate POS done")

def kill_pos():
    print ("\tTerminating POS..")
    ibof_proc.kill()
    ibof_proc.wait()
    print ("\tPOS Terminated!")

def expect_true(ret, notice):
    if ret ==  True:
        print ("\t" + notice +" Success")
    else:
        print ("\t" + notice + " Failed")

def expect_false(ret, notice):
    if ret == False:
        print ("\t" + notice + " Success")
    else:
        print ("\t" +  notice + " Failed")

def set_stdout_type(print_on):
    if print_on:
        stdout_type = subprocess.STDOUT
    else:
        stdout_type = subprocess.DEVNULL
    return stdout_type

def print_result(test_name, test_success):
    global test_index

    if test_success == True:
        result="Success"
    else:
        result="Failed"

    print("Test", test_index, test_name, result)
    test_index = test_index + 1

def print_start(test_name):
    print("Test", test_index, test_name, "Start")

def kill_and_wait(process_list):
    for proc in psutil.process_iter():
        try:
            for target_proc in process_list:
                if target_proc in proc.name():
                    proc.kill()
                    proc.wait()

            command_line = proc.cmdline()
            for command in command_line:
                if "poseidon_daemon.py" in command:
                    proc.kill()
                    proc.wait()

        except psutil.NoSuchProcess:
            pass



def clear_env():
    print ("\tCleanup - kill pos, fio, watchdog process")
    kill_and_wait(["poseidonos", "fio"])
    print ("\tCleanup Done")

test_index = 1
