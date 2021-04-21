#!/usr/bin/env python3

import argparse
import subprocess
import random
import string
import re
import psutil
import sys

sys.path.append("../lib/")
import cli
import json_parser

import TEST_LOG
import TEST_SETUP_POS
import TEST

def parse_arguments(args):
    parser = argparse.ArgumentParser(description='Test journal feature with SPO')
    parser.add_argument('-f', '--fabric_ip', default=TEST.traddr,\
            help='Set target IP, default: ' + TEST.traddr)
    parser.add_argument('-t', '--transport', default=TEST.trtype,
            help='Set transport, default: ' + TEST.trtype)
    parser.add_argument('-p', '--port', type=int, default=TEST.port,
            help='Set port, default: ' + str(TEST.port))
    parser.add_argument('-l', '--log_dir', default=TEST.log_dir,\
            help='Set path for log file, default: ' + TEST.log_dir)
    parser.add_argument('-q', '--quick_mode', default=TEST.quick_mode , action='store_true',\
            help='Enable quick test mode, default: ' + str(TEST.quick_mode))
    parser.add_argument('-d', '--dump_log_buffer', default=TEST.dump_log_buffer , action='store_true',\
            help='Enable dump log buffer during test, default: ' + str(TEST.dump_log_buffer))

    args = parser.parse_args()
    TEST.traddr = args.fabric_ip
    TEST.trtype = args.transport
    TEST.port = args.port
    TEST.log_dir = args.log_dir
    TEST.quick_mode = args.quick_mode
    TEST.dump_log_buffer = args.dump_log_buffer

def cleanup():
    TEST_SETUP_POS.cleanup_process()

def set_up(argv, test_name):
    parse_arguments(argv)
    TEST_LOG.setup_log(test_name)
    TEST_LOG.print_notice("[{} Started]".format(test_name))
    cleanup()

def tear_down(test_name):
    TEST_SETUP_POS.shutdown_pos()
    TEST_LOG.print_notice("[Test {} Completed]".format(test_name))

patterns = []

def create_new_pattern(volId):
    pattern = '\\\"' + ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(16)) + '\\\"'

    while len(patterns) <= volId:
        patterns.append([])

    patterns[volId].append(pattern)
    return patterns[volId].index(pattern)

def get_pattern(volId, index):
    return patterns[volId][index]

def get_latest_pattern(volId):
    return patterns[volId][-1]

units = {"B": 1, "K": 2**10, "M": 2**20, "G": 2**30, "KB": 2**10, "MB": 2**20, "GB": 2**30}

def parse_size(size):
    size = size.upper()
    if not re.match(r' ', size):
        size = re.sub(r'([KMG])', r' \1', size)

    number, unit = [string.strip() for string in size.split()]
    return int(int(number) * units[unit])

def get_available_memory():
    memory_usage_dict = dict(psutil.virtual_memory()._asdict())
    return memory_usage_dict['available']

def get_num_thread():
    available_memory_gb = get_available_memory() / 2.**30
    num_thread = int(available_memory_gb / TEST.EXPECTED_MEMORY_FIO)

    # Limit the number of threads that can be created if the amount of memory space availbale is low
    if num_thread > 10:
        num_thread = 10

    TEST_LOG.print_debug("Max number of threads will be set to {} (Current available memory: {} GB)".format(num_thread, round(available_memory_gb, 2)))
    return num_thread

def get_checkpoint_status():
    out = cli.send_request("wbt get_journal_status --array POSArray")
    if json_parser.get_response_code(out) != 0:
        return -1
    else:
        total_info = json_parser.get_data(out)
        return total_info["journalStatus"]["checkpointStatus"]["status"]

def get_log_buffer_size():
    out = cli.send_request("wbt get_journal_status --array POSArray")
    if json_parser.get_response_code(out) != 0:
        return -1
    else:
        total_info = json_parser.get_data(out)
        return total_info["journalStatus"]["logBufferStatus"]["logBufferSizeInBytes"]

def set_log_buffer_size(size):
    command = ("jq -r '.journal.buffer_size_in_mb |= {}' {} > tmp.conf && mv tmp.conf {}".format(size, TEST.config_path, TEST.config_path))
    subprocess.call(command, shell=True)

def set_journal(is_enable):
    command = ("jq -r '.journal.enable |= {}' {} > tmp.conf && mv tmp.conf {}".format(is_enable, TEST.config_path, TEST.config_path))
    subprocess.call(command, shell=True)

def is_journal_enabled():
    command = "cat /etc/pos/pos.conf | jq .journal.enable"
    out = subprocess.check_output(command, universal_newlines=True, shell=True)
    return out
