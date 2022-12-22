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


def verify_live_replication():
    lib.subproc.sync_run("mount /dev/nvme1n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    lib.subproc.async_run("/remote_home/chkang0912/ibofos/redis-server /remote_home/chkang0912/ibofos/test/system/ha/secondary.conf")
    time.sleep(1)

    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-cli -h 10.1.3.24 -p 8001 keys '*'")


def prepare_live_replication_test():
    lib.subproc.async_run("/remote_home/chkang0912/ibofos/redis-server /remote_home/chkang0912/ibofos/test/system/ha/primary.conf")
    time.sleep(1)

    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-benchmark -h 10.1.3.24 -p 8001 -t set -n 1000 -r 100000000")
    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-cli -h 10.1.3.24 -p 8001 set 2_live replication")
    lib.subproc.sync_run("pkill -SIGINT redis")
    time.sleep(2)
    lib.subproc.sync_run("cp /remote_home/chkang0912/ibofos/test/system/ha/data/image01.png /remote_home/chkang0912/ibofos/test/system/ha/pos_primary/live_replication.png")

    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")


def prepare_volume_copy_test():
    lib.subproc.async_run("/remote_home/chkang0912/ibofos/redis-server /remote_home/chkang0912/ibofos/test/system/ha/primary.conf")
    time.sleep(1)

    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-cli -h 10.1.3.24 -p 8001 set hello world")
    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-cli -h 10.1.3.24 -p 8001 set poseidon 2NodeHA")
    sync_run_and_print("/remote_home/chkang0912/redis/src/redis-cli -h 10.1.3.24 -p 8001 set 1_volume copy")
    lib.subproc.sync_run("pkill -SIGINT redis")
    time.sleep(2)
    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("mount /dev/nvme0n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")


def prepare_kernel_device_drive():
    print("* Preparing Kernel Device Drive")
    lib.subproc.sync_run("mkfs.xfs /dev/nvme0n1")
    lib.subproc.sync_run("mkfs.xfs /dev/nvme1n1")
    lib.subproc.sync_run("mount /dev/nvme0n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    lib.subproc.sync_run("mount /dev/nvme1n1 /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    lib.subproc.sync_run("cp /remote_home/chkang0912/ibofos/test/system/ha/data/image01.png /remote_home/chkang0912/ibofos/test/system/ha/pos_primary/before_volue_copy.png")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_primary")
    sync_run_and_print("ls -lah /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")
    lib.subproc.sync_run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary")


def clean_up(inits):
    lib.subproc.sync_run("pkill -9 redis")
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

    input("Please start Redis before volume copy")
    prepare_volume_copy_test()

    input("Wait volume copy finish")

    input("Please start Redis before live replication")
    prepare_live_replication_test()

    input("Wait live replication finish")
    verify_live_replication()

    return data
