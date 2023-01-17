import iogen
import lib
import node
import time
import traceback
import random
import string
import os
import subprocess


def sync_run_and_print(command):
    result = lib.subproc.sync_run(command)
    print(result)


def verify_volume_copy():
    # lib.subproc.sync_run("pkill -9 replicator")
    # time.sleep(2)
    # lib.subproc.sync_run("mount /dev/nvme0n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("mount /dev/nvme1n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    sync_run_and_print("feh /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary/before_volue_copy.png")
    sync_run_and_print("feh /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary/during_volue_copy.png")


def insert_io_during_volume_copy():
    lib.subproc.sync_run("cp /remote_home/chkang0912/ibofos/test/system/ha/data/image02.png /remote_home/chkang0912/ibofos/test/system/ha/pos_primary/during_volue_copy.png")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")


def prepare_volume_copy_test():
    lib.subproc.sync_run("cp /remote_home/chkang0912/ibofos/test/system/ha/data/image01.png /remote_home/chkang0912/ibofos/test/system/ha/pos_primary/before_volue_copy.png")
    sync_run_and_print("df -h")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    sync_run_and_print("feh /remote_home/chkang0912/ibofos/test/system/ha/pos_primary/before_volue_copy.png")
    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    lib.subproc.sync_run("mount /dev/nvme0n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")


def prepare_kernel_device_drive():
    print("* Preparing Kernel Device Drive")
    lib.subproc.sync_run("mkfs.xfs /dev/nvme0n1")
    lib.subproc.sync_run("mkfs.xfs /dev/nvme1n1")
    lib.subproc.sync_run("mount /dev/nvme0n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("mount /dev/nvme1n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")


def clean_up(inits):
    lib.subproc.sync_run("pkill -9 replicator")
    lib.subproc.sync_run("rm -rf /var/log/pos/*")
    result = subprocess.run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary", shell=True, stdout=subprocess.PIPE)
    result = subprocess.run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary", shell=True, stdout=subprocess.PIPE)
    for init_dict in inits:
        for subsys in init_dict["TARGETs"][0]["SUBSYSTEMs"]:
            nqn_index = subsys["NQN_INDEX"]
            nqn = f"{subsys['NQN_PREFIX']}{nqn_index:03d}"
            lib.subproc.sync_run(f"nvme disconnect -n {nqn} ")


def play(tgts, inits, scenario, timestamp, data):
    clean_up(inits)
    try:  # Prepare sequence
        node_manager = node.NodeManager(tgts, inits)
        targets, initiators = node_manager.initialize()

    except Exception as e:
        lib.printer.red(traceback.format_exc())
        return data
    prepare_kernel_device_drive()
    prepare_volume_copy_test()

    input("Please start volume copy")

    input("Insert additional IO during volume copy")
    insert_io_during_volume_copy()

    input("Wait volume copy finish")

    verify_volume_copy()

    return data
