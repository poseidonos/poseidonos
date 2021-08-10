#!/usr/bin/env python3

import TEST_SETUP_POS
import TEST_LOG
import TEST
import sys
import os
import subprocess

sys.path.append("../lib/")
import cli  # noqa: E402
import json_parser  # noqa: E402


def flush_gcov():
    out = cli.wbt_request("flush_gcov", "")
    if json_parser.get_response_code(out) != 0:
        TEST_LOG.print_err("* Gcov flush failed, check WBT option")
    elif json_parser.get_data_code(out) != 0:
        TEST_LOG.print_info("* Gcov flush failed, check gcov option")
    else:
        TEST_LOG.print_info("* Flush gcov completed")


def dump_journal(arrayId, dump_file):
    if TEST.dump_log_buffer == False:
        return

    fd = get_fd(arrayId, TEST.log_buffer_filename)
    if fd == -1:
        TEST_LOG.print_err("* Log buffer dump failed (cannot get fd), check WBT option")
        return

    file_size = get_file_size(arrayId, fd)
    if file_size == -1:
        TEST_LOG.print_err("* Log buffer dump failed (cannot get filesize), check WBT option")
        return

    result = dump_log_buffer(arrayId, fd, file_size, dump_file)
    if result == -1:
        TEST_LOG.print_err("* Log buffer dump failed, check WBT option")
        return

    TEST_LOG.print_info("* Dump log buffer completed")


def dump_vsamap(arrayId, volid, dump_file):
    if TEST.dump_map == False:
        return

    volname = TEST_SETUP_POS.get_volname(volid)
    arguments = "--name " + volname + " --array " + TEST_SETUP_POS.get_arrayname(arrayId) + " --output " + dump_file
    out = cli.wbt_request("read_vsamap", arguments)
    if json_parser.get_response_code(out) != 0:
        return -1


def dump_stripemap(dump_file):
    if TEST.dump_map == False:
        return

    arguments = "--array " + TEST_SETUP_POS.get_arrayname(arrayId) + " --output " + dump_file
    out = cli.wbt_request("read_stripemap", arguments)
    if json_parser.get_response_code(out) != 0:
        return -1


def get_fd(arrayId, filename):
    filesInfo = "filesInfo.json"
    arguments = " --array " + TEST_SETUP_POS.get_arrayname(arrayId) + " --output " + filesInfo
    out = cli.wbt_request("mfs_dump_files_list", arguments)
    if json_parser.get_response_code(out) != 0:
        return -1

    fd = -1
    with open(filesInfo) as json_file:
        filesList = json_parser.get_value_from_json(json_file, "filesInfoList")
        for f in filesList:
            if f['fileName'] == filename:
                fd = f['fd']
    return fd


def get_file_size(arrayId, fd):
    arguments = "--fd " + str(fd) + " --array " + TEST_SETUP_POS.get_arrayname(arrayId)
    out = cli.wbt_request("mfs_get_file_size", arguments)
    if json_parser.get_response_code(out) != 0:
        return -1
    else:
        return json_parser.get_data_code(out)


def start_core_dump(trigger_option):
    TEST_LOG.print_err("* Try to dump core file. (option: {})".format(trigger_option))
    core_dump_cmd = TEST.pos_root + "/tool/dump/trigger_core_dump.sh"
    subprocess.call(core_dump_cmd + " " + trigger_option, shell=True)


def dump_log_buffer(arrayId, fd, filesize, dumpfile):
    arguments = "--fd " + str(fd) + " --offset 0 --count " + str(filesize) + " --array " + TEST_SETUP_POS.get_arrayname(arrayId) + " --output " + dumpfile
    out = cli.wbt_request("mfs_read_file", arguments)
    if json_parser.get_response_code(out) != 0:
        return -1
    return 0
