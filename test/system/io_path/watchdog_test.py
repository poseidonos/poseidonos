#!/usr/bin/env python3

import subprocess, os, psutil, time
import common_test_lib
import argparse

#######################################################################################
# edit test parameters into these lists to run different workloads
pos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
watchdog_path = pos_root + "tool/watchdog/poseidon_daemon.py"
default_timeout = 30
#######################################################################################


def find_pos():
    for proc in psutil.process_iter():
        if "poseidonos" in proc.name():
            return True
    return False


def wait_pos_execution():
    print("\tWait Poseidon OS execution")
    sec = 1
    while find_pos() is False:
        print ("\t\t", sec, " seconed(s)..")
        time.sleep(1)
        sec = sec + 1
        if int(args.timeout) < sec:
            print("\tTimeout")
            return False
    print ("\tExecuted")
    return True


def execute_watchdog(deamon=0, log_path="watchdog.log"):
    global watchdog_proc
    if find_log_file(log_path):
        os.remove(log_path)
    FNULL = open(os.devnull,'w')
    print ("\tInvoke watchdog process")
    watchdog_proc = subprocess.Popen([watchdog_path,
        "-d", str(deamon),
        "-l", log_path,
        "-p", pos_root],
        stdout=FNULL, stderr=FNULL)
    FNULL.close


def wait_deamonize():
    print ("\tWait Deamonize")
    watchdog_proc.wait()
    print ("\tDeamonize Done")


def find_log_file(log_path):
    return os.path.exists(log_path)


def run_test():
    test_name = "Watchdog run test"
    common_test_lib.print_start(test_name)
    common_test_lib.clear_env()
    execute_watchdog(deamon = 0)
    executed = wait_pos_execution()
    common_test_lib.terminate_pos(pos_root, subprocess.DEVNULL)
    common_test_lib.clear_env()
    common_test_lib.print_result(test_name, executed)
    return executed


def deamonize_test():
    test_name = "Deamonize run test"
    common_test_lib.print_start(test_name)
    common_test_lib.clear_env()
    execute_watchdog(deamon = 1)
    wait_deamonize()
    executed = wait_pos_execution()
    common_test_lib.terminate_pos(pos_root,  subprocess.DEVNULL)
    common_test_lib.clear_env()
    common_test_lib.print_result(test_name, executed)
    return executed


def log_path_test():
    test_name = "Log path test"
    common_test_lib.print_start(test_name)
    common_test_lib.clear_env()
    log_file_name = "watchdog_test.log"
    execute_watchdog(deamon = 1, log_path = log_file_name)
    wait_deamonize()
    found = find_log_file(log_file_name)
    common_test_lib.terminate_pos(pos_root, subprocess.DEVNULL)
    common_test_lib.clear_env()
    common_test_lib.print_result(test_name, found)
    return found


def parse_arguments():
    parser = argparse.ArgumentParser(description='Test watchdog')
    parser.add_argument('-t', '--timeout', default=default_timeout,
            help='Set watchdog timeout, default: ' + str(default_timeout))
    global args
    args = parser.parse_args()
    print (args)


if __name__ == "__main__":
    common_test_lib.clear_env()
    parse_arguments()
    success = run_test()
    success &= deamonize_test()
    success &= log_path_test()
    if success:
        exit(0)
    else:
        exit(-1)
