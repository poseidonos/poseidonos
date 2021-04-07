#!/usr/bin/env python3

import sys
import os
import subprocess

sys.path.append("../lib/")
import cli
import json_parser

import TEST
import TEST_LOG

def flush_gcov():
    out = cli.send_request("wbt flush_gcov")
    if json_parser.get_response_code(out) != 0:
        TEST_LOG.print_err("* Gcov flush failed, check WBT option")
    elif json_parser.get_data_code(out) != 0:
        TEST_LOG.print_info("* Gcov flush failed, check gcov option") 
    else:
        TEST_LOG.print_info("* Flush gcov completed")

def dump_journal(dump_file):
    if TEST.dump_log_buffer == False:
        return

    fd = get_fd(TEST.log_buffer_filename)
    if fd == -1:
        TEST_LOG.print_err("* Log buffer dump failed (cannot get fd), check WBT option")
        return

    file_size = get_file_size(fd)
    if file_size == -1:
        TEST_LOG.print_err("* Log buffer dump failed (cannot get filesize), check WBT option")
        return

    result = dump_log_buffer(fd, file_size, dump_file)
    if result == -1:
        TEST_LOG.print_err("* Log buffer dump failed, check WBT option")
        return

    TEST_LOG.print_info("* Dump log buffer completed")

def get_fd(filename):
    filesInfo = "filesInfo.json"
    out = cli.send_request("wbt mfs_dump_files_list --output " + filesInfo)
    if json_parser.get_response_code(out) != 0:
        return -1

    fd = -1
    with open(filesInfo) as json_file:
        filesList = json_parser.get_value_from_json(json_file, "filesInfoList")
        for f in filesList:
            if f['fileName'] == filename:
                fd = f['fd']
    return fd

def get_file_size(fd):
    out = cli.send_request("wbt mfs_get_file_size --fd " + str(fd))
    if json_parser.get_response_code(out) != 0:
        return -1
    else:
        return json_parser.get_data_code(out)


def dump_log_buffer(fd, filesize, dumpfile):
    out = cli.send_request("wbt mfs_read_file --fd " + str(fd) + " --offset 0 --count " + str(filesize) + " --output " + dumpfile)
    if json_parser.get_response_code(out) != 0:
        return -1
    return 0