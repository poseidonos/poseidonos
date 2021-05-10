#!/usr/bin/env python3

import sys
import os

sys.path.append("../lib/")
sys.path.append("../io_path/")

import subprocess

import pos
import pos_constant

import cli
import json_parser
import spdk_rpc

import TEST
import TEST_LIB
import TEST_LOG
import TEST_DEBUGGING
import TEST_RUN_POS

ARRAYNAME = "POSArray"

def shutdown_pos():
    TEST_RUN_POS.block_pos_crash_detection()

    TEST_LOG.print_info("* Exiting POS")
    out = cli.unmount_array(ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount pos")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    out = cli.exit_pos()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to exit pos")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_RUN_POS.wait_for_pos_shutdown()

def mbr_reset():
    cli.mbr_reset()

def clean_bringup():
    TEST_LOG.print_info("* POS clean bringup")

    if (os.path.isfile(TEST.mockfile)):
        os.remove(TEST.mockfile)

    TEST_RUN_POS.start_pos()
    subprocess.call(["sleep", "3"])

    setup()
    mbr_reset()
    create_array()
    mount_array()

    TEST_LOG.print_info("* Fininshed bringup")

def dirty_bringup(dump=0):
    TEST_LOG.print_info("* POS dirty bringup")

    TEST_RUN_POS.start_pos()
    subprocess.call(["sleep", "3"])

    setup()
    mount_array()

    TEST_DEBUGGING.dump_journal("LogBuffer_AfterSPO")
    TEST_LOG.print_info("* Fininshed bringup")

def setup():
    command=""

    if TEST.trtype == "tcp":
        command += " -b 64 -n 4096"
    elif TEST.trtype == "rdma":
        command += " -u 131072"

    spdk_rpc.send_request("nvmf_create_transport -t " + TEST.trtype + command)
    spdk_rpc.send_request("bdev_malloc_create -b uram0 1024 512")
#pmem
#    spdk_rpc.send_request("bdev_pmem_create_pool /mnt/pmem0/pmem_pool 1024 512")
#    spdk_rpc.send_request("bdev_pmem_create /mnt/pmem0/pmem_pool -n pmem0")

    out = cli.scan_device()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to scan device")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Setup POS")

def create_array():
    out = cli.create_array("uram0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "unvme-ns-3", ARRAYNAME, "")
#   pmem
#    out = cli.create_array("pmem0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "unvme-ns-3", ARRAYNAME, "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create array")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* Array created")

def mount_array():
    out = cli.mount_array(ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount pos")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* POS mounted")

def unmount_array():
    out = cli.unmount_array(ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount pos")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* POS unmounted")

def create_subsystem(volumeId):
    out = spdk_rpc.send_request("nvmf_create_subsystem " + TEST.NQN + str(volumeId) \
        + " -a -s POS0000000000000" + str(volumeId) + " -d POS_VOLUME_EXTENTION")
    if out != 0:
        TEST_LOG.print_err("Failed to create subsystem")
        sys.exit(1)

def get_volname(volumeId):
    return "vol"+str(volumeId)

def create_volume(volumeId, subnqn=""):
    out = cli.create_volume(get_volname(volumeId), str(TEST.volSize), "0", "0", ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} created".format(volumeId))
    mount_volume(volumeId)

def mount_volume(volumeId):
    out = spdk_rpc.send_request("nvmf_subsystem_add_listener " + TEST.NQN + str(volumeId) \
        + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to create volume")
        sys.exit(1)

    out = cli.mount_volume(get_volname(volumeId), ARRAYNAME, TEST.NQN + str(volumeId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} mounted".format(volumeId))

def unmount_volume(volumeId):
    out = cli.unmount_volume(get_volname(volumeId), ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    out = spdk_rpc.send_request("nvmf_subsystem_remove_listener " + TEST.NQN + str(volumeId) \
        + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} unmounted".format(volumeId))

def delete_volume(volumeId):
    out = cli.delete_volume(get_volname(volumeId), ARRAYNAME)
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to delete volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    out = spdk_rpc.send_request("nvmf_delete_subsystem " + TEST.NQN + str(volumeId))
    if out != 0:
        TEST_LOG.print_err("Failed to delete volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} deleted".format(volumeId))

def backup_nvram():
    backup_execution = TEST.pos_root + "script/backup_latest_hugepages_for_uram.sh"
    with open(TEST.output_log_path, "a") as log_file:
        backup_proc = subprocess.Popen([backup_execution], stdout=log_file, stderr=log_file)

    if backup_proc.wait() != 0:
        TEST_LOG.print_err("Failed to backup uram")
        sys.exit(1)

def trigger_spor():
    TEST_DEBUGGING.dump_journal("LogBuffer_BeforeSPO")
    TEST_DEBUGGING.flush_gcov()

    TEST_RUN_POS.block_pos_crash_detection()
    TEST_RUN_POS.kill_pos()

    TEST_LIB.kill_process("fio")
    backup_nvram()


def cleanup_pos_logs():
    os.system('rm -rf /etc/pos/core/*')
    os.system('rm -rf /var/log/pos/*')
