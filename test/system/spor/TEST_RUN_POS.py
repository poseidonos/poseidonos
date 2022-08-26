#!/usr/bin/env python3

import os
import signal
import subprocess
import sys
import time

import TEST
import TEST_LOG
import TEST_LIB
import TEST_DEBUGGING

POSBIN = "poseidonos"
detect_pos_crash = False


def is_pos_alive():
    isAlive = False
    if TEST.run_pos_manually is True:
        isAlive = TEST_LIB.find_process(POSBIN)
    else:
        poll_result = pos_proc.poll()
        if poll_result is None:
            isAlive = True
    return isAlive


def chldSignal_handler(sig, frame):
    global pos_proc
    global detect_pos_crash

    if detect_pos_crash:
        if is_pos_alive() is False:
            TEST_LOG.print_err("* POS terminated unexpectedly")
            TEST_LIB.kill_process("fio")
            detect_pos_crash = False

            TEST_DEBUGGING.start_core_dump("crashed")
            sys.exit(1)


def quitSignal_handler(sig, frame):
    block_pos_crash_detection()

    TEST_LOG.print_err("* Test force stop signal received")
    TEST_LIB.kill_process("fio")
    TEST_DEBUGGING.start_core_dump("triggercrash")
    sys.exit(1)


def block_pos_crash_detection():
    global detect_pos_crash
    detect_pos_crash = False


def start_pos():
    global pos_proc
    global detect_pos_crash

    if TEST.run_pos_manually is True:
        input("Please start pos manually and enter")
    else:
        TEST_LOG.print_info("* Starting POS")
        pos_execution = TEST.pos_root + "bin/" + POSBIN
        with open(TEST.pos_log_path, "a") as log_file:
            pos_proc = subprocess.Popen(pos_execution, stdout=log_file, stderr=log_file)

        time.sleep(10)
        TEST_LOG.print_info("* Starting Telemetry")
        telemetry_execution = TEST.pos_root + "bin/poseidonos-cli"
        with open(TEST.pos_log_path, "a") as log_file:
            telemetry_proc = subprocess.Popen([telemetry_execution, "telemetry", "start"], stdout=log_file, stderr=log_file)

    subprocess.call(["sleep", "50"])

    detect_pos_crash = True

    signal.signal(signal.SIGCHLD, chldSignal_handler)
    signal.signal(signal.SIGQUIT, quitSignal_handler)


def kill_pos():
    block_pos_crash_detection()

    if TEST.run_pos_manually is True:
        TEST_LIB.kill_process(POSBIN)
    else:
        pos_proc.kill()
        pos_proc.wait()

        TEST_LOG.print_info("* POS killed")


def wait_for_pos_shutdown():
    global detect_pos_crash

    if TEST.run_pos_manually is True:
        while TEST_LIB.find_process(POSBIN) is True:
            pass
    else:
        pos_proc.wait()
        if pos_proc.returncode != 0:
            TEST_LOG.print_err("* POS terminated unexpectedly")
            TEST_DEBUGGING.start_core_dump("crashed")
            sys.exit(1)


def cleanup_process():
    os.system('rm -rf /dev/shm/ibof_nvmf_trace*')

    TEST_LIB.kill_process(POSBIN)
    TEST_LIB.kill_process("fio")
