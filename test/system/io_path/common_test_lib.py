#!/usr/bin/env python3

import subprocess
import sys
import psutil


def start_pos(log_path, pos_root):
    global pos_proc
    print("\tStarting POS .. (log path: " + log_path + ")")
    pos_execution = pos_root + "bin/poseidonos"
    with open(log_path, "w") as output_file:
        pos_proc = subprocess.Popen(pos_execution, stdout=output_file, stderr=output_file)
    subprocess.call(["sleep", "10"])
    print("\tPOS Started!")

    print("\tStarting Telemetry .. (log path: " + log_path + ")")
    telemetry_execution = pos_root + "bin/poseidonos-cli"
    with open(log_path, "w") as output_file:
        telemetry_proc = subprocess.Popen([telemetry_execution, "telemetry", "start"], stdout=output_file, stderr=output_file)
    subprocess.call(["sleep", "10"])
    print("\tTelemetry Started!")

def bringup_pos(**args):
    args['volume_cnt'] = 1
    args['subsystem_cnt'] = 1
    bringup_multiple_volume(**args)


def bringup_multiple_volume(**args):
    start_pos(args['log_path'], args['pos_root'])
    pos_bringup = args['pos_root'] + \
        "/test/system/io_path/setup_ibofos_nvmf_volume.sh"
    print("\tBringup POS")
    if 'clean' not in args:
        args['clean'] = 1
    if 'volume_size' not in args:
        args['volume_size'] = 2147483648

    subprocess.call([pos_bringup,
                     "-t", args['transport'],
                     "-a", args['target_ip'],
                     "-v", str(args['volume_cnt']),
                     "-s", str(args['subsystem_cnt']),
                     "-S", str(args['volume_size']) + "B",
                     "-c", str(args['clean'])],
                    stdout=args['stdout_type'], stderr=subprocess.STDOUT)
    print("\tBringup Done")


def scan_device(pos_root, stdout_type):
    print("\tScan Device")
    pos_cli = pos_root + "bin/poseidonos-cli"
    subprocess.call([pos_cli, "device", "scan"], stdout=stdout_type, stderr=subprocess.STDOUT)
    print("\tScan Device Done")


def reset_mbr(pos_root, stdout_type):
    print("\tReset MBR")
    pos_cli = pos_root + "bin/poseidonos-cli"
    subprocess.call([pos_cli, "devel", "resetmbr"], stdout=stdout_type, stderr=subprocess.STDOUT)
    print("\tReset MBR Done")


def create_uram(pos_root, stdout_type, uram_name):
    print("\tCreate Uram")
    pos_cli = pos_root + "bin/poseidonos-cli"
    subprocess.call([pos_cli, "device", "create", "-d", uram_name, "--num-blocks", "2097152", "--block-size", "512", "--device-type", "uram"],
                    stderr=subprocess.STDOUT)
    print("\tCreate Uram Done")


def create_and_mount_array(pos_root, stdout_type, uram_list, user_devices, array_list):
    print("\tCreate and Mount Array")
    array_count = len(user_devices)
    print("\tarray count : ", array_count)
    pos_cli = pos_root + "bin/poseidonos-cli"
    for i in range(0, array_count):
        target_device_list = ""
        for j in range(0, len(user_devices[i]) - 1):
            target_device_list += "unvme-ns-" + str(user_devices[i][j]) + ","
        target_device_list += "unvme-ns-" + str(user_devices[i][j + 1])
        print("\ttarget device list: ", target_device_list, uram_list[i], array_list[i])

        subprocess.call([pos_cli, "array", "create", "-b", uram_list[i], "-d", target_device_list, "-a", array_list[i], "--raid", "RAID5"],
                        stdout=stdout_type, stderr=subprocess.STDOUT)
        subprocess.call([pos_cli, "array", "mount", "-a", array_list[i]], stdout=stdout_type, stderr=subprocess.STDOUT)
    print("\tCreate and Mount Array Done")


def create_volume(pos_root, stdout_type, volume_name, array_name):
    print("\tCreate Volume")
    pos_cli = pos_root + "bin/poseidonos-cli"
    subprocess.call([pos_cli, "volume", "create", "-v", volume_name, "--size", "2GB", "-a", array_name],
                    stdout=stdout_type, stderr=subprocess.STDOUT)
    print("\tCreate Volume Done")


def terminate_pos(pos_root, stdout_type, array_list=""):
    print("\tTerminate POS")
    if 'pos_proc' in globals():
        pos_cli = pos_root + "bin/poseidonos-cli"
        if not array_list:
            subprocess.call([pos_cli, "array", "unmount", "--array-name", "POSArray", "--force"],
                            stdout=stdout_type, stderr=subprocess.STDOUT)
        else:
            for i in range(0, len(array_list)):
                subprocess.call([pos_cli, "array", "unmount", "--array-name", array_list[i], "--force"],
                                stdout=stdout_type, stderr=subprocess.STDOUT)
        subprocess.call([pos_cli, "system", "stop", "--force"],
                        stdout=stdout_type, stderr=subprocess.STDOUT)
        pos_proc.wait()
    print("\tTerminate POS done")


def kill_pos():
    print("\tTerminating POS..")
    pos_proc.kill()
    pos_proc.wait()
    print("\tPOS Terminated!")


def expect_true(ret, notice):
    if ret is True:
        print("\t" + notice + " Success")
    else:
        print("\t" + notice + " Failed")


def expect_false(ret, notice):
    if ret is False:
        print("\t" + notice + " Success")
    else:
        print("\t" + notice + " Failed")


def set_stdout_type(print_on):
    if print_on:
        stdout_type = subprocess.STDOUT
    else:
        stdout_type = subprocess.DEVNULL
    return stdout_type


def print_result(test_name, test_success):
    global test_index

    if test_success is True:
        result = "Success"
    else:
        result = "Failed"

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
    print("\tCleanup - kill pos, fio, watchdog process")
    kill_and_wait(["poseidonos", "fio"])
    print("\tCleanup Done")


test_index = 1
