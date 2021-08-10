#!/usr/bin/env python3

import subprocess
import sys
import os

import TEST_RUN_POS
import TEST_DEBUGGING
import TEST_LOG
import TEST_LIB
import TEST

sys.path.append("../lib/")
sys.path.append("../io_path/")
import spdk_rpc  # noqa: E402
import json_parser  # noqa: E402
import cli  # noqa: E402

ARRAYNAME = "POSArray"


def shutdown_pos(numArray):
    TEST_RUN_POS.block_pos_crash_detection()

    TEST_LOG.print_info("* Exiting POS")

    for arrayId in range(numArray):
        unmount_array(arrayId)
        delete_array(arrayId)

    out = cli.exit_pos()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to exit pos")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_RUN_POS.wait_for_pos_shutdown()


def mbr_reset():
    scan_device()
    cli.mbr_reset()


def clean_bringup(arrays=[0]):
    TEST_LOG.print_info("* POS clean bringup")

    if (os.path.isfile(TEST.mockfile)):
        os.remove(TEST.mockfile)

    TEST_RUN_POS.start_pos()

    mbr_reset()
    create_nvmf()

    for arrayId in arrays:
        add_array(arrayId)

    TEST_LOG.print_info("* Fininshed bringup")


def dirty_bringup(arrays=[0]):
    TEST_LOG.print_info("* POS dirty bringup")

    TEST_RUN_POS.start_pos()

    create_nvmf()

    for uramId in arrays:
        create_uram(uramId)
        # create_pram()
    scan_device()

    for arrayId in arrays:
        mount_array(arrayId)
        TEST_DEBUGGING.dump_journal(arrayId, "LogBuffer_AfterSPO")

    TEST_LOG.print_info("* Fininshed bringup")


def scan_device():
    out = cli.scan_device()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to scan device")
        TEST_LOG.print_debug(out)
        sys.exit(1)


# TODO(cheolho.kang): Seperate array setup method from setup function
def create_nvmf():
    command = ""

    if TEST.trtype == "tcp":
        command += " -b 64 -n 4096"
    elif TEST.trtype == "rdma":
        command += " -u 131072"

    spdk_rpc.send_request("nvmf_create_transport -t " + TEST.trtype + command)

    TEST_LOG.print_info("* Setup POS")


def get_uramname(uramId):
    return "uram" + str(uramId)


def create_pram():
    spdk_rpc.send_request("bdev_pmem_create_pool /mnt/pmem0/pmem_pool 1024 512")
    spdk_rpc.send_request("bdev_pmem_create /mnt/pmem0/pmem_pool -n pmem0")


def create_uram(uramId):
    spdk_rpc.send_request("bdev_malloc_create -b " + get_uramname(uramId) + " 1024 512")
    # cli.create_uram(get_uramname(uramId), "512", "2")


def get_device_name(arrayId):
    dataDevice = ""
    spareDevice = ""

    num_devices = TEST.num_data_ssds_per_array + TEST.num_spare_ssds_per_array
    for index in range(num_devices):
        deviceName = "unvme-ns-" + str(index + arrayId * num_devices)
        if index < TEST.num_data_ssds_per_array:
            dataDevice += ("," + deviceName)
        else:
            spareDevice += ("," + deviceName)

    dataDevice = dataDevice[1:]
    spareDevice = spareDevice[1:]

    return (dataDevice, spareDevice)


def get_num_ssd():
    out = cli.list_device()
    numSSD = 0
    for device in json_parser.get_data(out)['devicelist']:
        if device['type'] == "SSD":
            numSSD += 1
    return numSSD


def get_max_num_array():
    return get_num_ssd() / (TEST.num_data_ssds_per_array + TEST.num_spare_ssds_per_array)


def create_array(arrayId):
    if get_max_num_array() < arrayId + 1:
        TEST_LOG.print_info("Not enough SSD to create array. Therefore this test will be bypassed. Number of SSD is {}".format(get_num_ssd()))
        sys.exit(0)

    dataDevice, spareDevice = get_device_name(arrayId)
    out = cli.create_array(get_uramname(arrayId), dataDevice, spareDevice, get_arrayname(arrayId), "")
#   pmem
#   out = cli.create_array("pmem0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "unvme-ns-3", ARRAYNAME, "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create array{}".format(arrayId))
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* Array created")
    mount_array(arrayId)


def add_array(arrayId):
    create_uram(arrayId)
    # create_pram()
    scan_device()

    create_array(arrayId)


def mount_array(arrayId=0):
    out = cli.mount_array(get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount array{}".format(arrayId))
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* {} mounted".format(get_arrayname(arrayId)))


def unmount_array(arrayId=0):
    out = cli.unmount_array(get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount array{}".format(arrayId))
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* {} unmounted".format(get_arrayname(arrayId)))


def delete_array(arrayId):
    out = cli.delete_array(get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to delete array{}".format(arrayId))
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* {} delete".format(get_arrayname(arrayId)))


def create_subsystem(arrayId, volumeId):
    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    out = spdk_rpc.send_request("nvmf_create_subsystem " + TEST.NQN + str(subsystemId)
                                + " -a -s POS0000000000000" + str(subsystemId) + " -d POS_VOLUME_EXTENTION")
    if out != 0:
        TEST_LOG.print_err("Failed to create subsystem")
        sys.exit(1)


def get_arrayname(arrayId):
    return ARRAYNAME + str(arrayId)


def get_volname(volumeId):
    return "vol" + str(volumeId)


def create_volume(arrayId, volumeId, volSize=TEST.volSize):
    out = cli.create_volume(get_volname(volumeId), str(volSize), "0", "0", get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} created, size {}".format(volumeId, volSize))
    mount_volume(arrayId, volumeId)


def mount_volume(arrayId, volumeId):
    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    out = spdk_rpc.send_request("nvmf_subsystem_add_listener " + TEST.NQN + str(subsystemId)
                                + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to create volume")
        sys.exit(1)

    out = cli.mount_volume(get_volname(volumeId), get_arrayname(arrayId), TEST.NQN + str(subsystemId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} mounted".format(volumeId))


def unmount_volume(arrayId, volumeId):
    out = cli.unmount_volume(get_volname(volumeId), get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    out = spdk_rpc.send_request("nvmf_subsystem_remove_listener " + TEST.NQN + str(subsystemId)
                                + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} unmounted".format(volumeId))


def delete_volume(arrayId, volumeId):
    out = cli.delete_volume(get_volname(volumeId), get_arrayname(arrayId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to delete volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    out = spdk_rpc.send_request("nvmf_delete_subsystem " + TEST.NQN + str(subsystemId))
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


def trigger_spor(numArray=1):
    for arrayId in range(numArray):
        TEST_DEBUGGING.dump_journal(arrayId, "LogBuffer_BeforeSPO")

    TEST_DEBUGGING.flush_gcov()

    TEST_RUN_POS.block_pos_crash_detection()
    TEST_RUN_POS.kill_pos()

    TEST_LIB.kill_process("fio")
    backup_nvram()


def cleanup_pos_logs():
    os.system('rm -rf /etc/pos/core/*')
    os.system('rm -rf /var/log/pos/*')
