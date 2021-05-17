#!/usr/bin/env python3

import os
from datetime import datetime

import TEST


class bcolors:
    RED = "\033[1;31m"
    GREEN = "\033[1;32m"
    BLUE = "\033[1;34m"
    RESET = "\033[0;0m"


def setup_log(test_name):
    if not os.path.isdir(TEST.log_dir):
        os.mkdir(TEST.log_dir)
    if TEST.log_dir[-1] != "/":
        TEST.log_dir += "/"

    current_log_dir = TEST.log_dir + TEST.log_date + "_" + test_name
    os.mkdir(current_log_dir)
    TEST.test_log_path = current_log_dir + "/" + test_name + ".log"
    TEST.output_log_path = current_log_dir + "/" + test_name + "_out.log"
    TEST.pos_log_path = current_log_dir + "/pos.log"

    print_info("Test log directory : " + current_log_dir)


def _print_msg(msg, level="DEBUG", color=""):
    if level != "DEBUG":
        if TEST.print_log_with_color == True:
            print_msg = color + msg + bcolors.RESET
        else:
            print_msg = msg
        print(print_msg)

    spor_logger = open(TEST.test_log_path, "a")
    spor_logger.write("[" + datetime.today().strftime("%d %b %H:%M:%S") + "][" + level + "] " + msg + "\n")


def print_err(errmsg, color=bcolors.RED):
    _print_msg(errmsg, "ERROR", color)


def print_info(msg, color=bcolors.BLUE):
    _print_msg(msg, "INFO", color)


def print_notice(msg, color=bcolors.GREEN):
    _print_msg(msg, "NOTICE", color)


def print_debug(msg):
    _print_msg(msg, "DEBUG")
