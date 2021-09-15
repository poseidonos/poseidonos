#!/usr/bin/env python3

import subprocess
import sys

POS_ROOT = '../../../'
POS_CLI = POS_ROOT + "bin/poseidonos-cli"


def send_request(msg):
    cli_req = POS_CLI + " --json-res "
    out = subprocess.check_output(cli_req + msg, universal_newlines=True, shell=True)
    return out


def exit_pos():
    return send_request("system stop --force")


def get_pos_info():
    return send_request("system info")


def get_version():
    return send_request("system version")


def create_array(buffer, data, spare, array, raid):
    param_str = ""
    if (buffer != ""):
        param_str += "-b " + buffer
    if (data != ""):
        param_str += " -d " + data
    if spare != "":
        param_str += " -s " + spare
    param_str += " --array-name " + array
    if raid != "":
        param_str += " --raid " + raid
    return send_request("array create " + param_str)


def delete_array(array):
    param_str = ""
    param_str += "--array-name " + array
    return send_request("array delete " + param_str + " --force")


def mount_array(array):
    param_str = ""
    param_str += "--array-name " + array
    return send_request("array mount " + param_str)


def unmount_array(array):
    param_str = ""
    param_str += "--array-name " + array
    return send_request("array unmount " + param_str + " --force")


def list_array():
    return send_request("array list")


def array_info(array):
    return send_request("array list --array-name " + array)


def add_device(spare, array):
    param_str = "--spare " + spare
    param_str += " --array-name " + array
    return send_request("array addspare " + param_str)


def remove_device(spare, array):
    param_str = "--spare " + spare
    param_str += " --array-name " + array
    return send_request("array rmspare " + param_str)


def create_volume(vol_name, size, iops, bw, array):
    param_str = "--volume-name " + vol_name + " --size " + size + "B"
    if iops != "":
        param_str += " --maxiops " + iops
    if bw != "":
        param_str += " --maxbw " + bw
    param_str += " --array-name " + array

    return send_request("volume create " + param_str)


def delete_volume(vol_name, array):
    param_str = "--volume-name " + vol_name
    param_str += " --array-name " + array
    return send_request("volume delete " + param_str + " --force")


def mount_volume(vol_name, array, subnqn):
    param_str = "--volume-name " + vol_name
    param_str += " --array-name " + array
    if subnqn != "":
        param_str += " --subnqn " + subnqn + " --force"
    return send_request("volume mount " + param_str)


def unmount_volume(vol_name, array):
    param_str = "--volume-name " + vol_name
    param_str += " --array-name " + array
    return send_request("volume unmount " + param_str + " --force")


def list_volume(array):
    param_str = ""
    param_str += " --array-name " + array
    return send_request("volume list " + param_str)


def update_volume_qos(vol_name, iops, bw, array):
    param_str = "--volume-name " + vol_name
    if iops != "":
        param_str += " --maxiops " + iops
    if bw != "":
        param_str += " --maxbw " + bw
    param_str += " --array-name " + array

    return send_request("qos create " + param_str)


def rename_volume(vol_name, new_name, array):
    param_str = "--volume-name " + vol_name + " --new-volume-name " + new_name
    param_str += " --array-name " + array

    return send_request("volume rename " + param_str)


def scan_device():
    return send_request("device scan")


def mbr_reset():
    out = send_request("devel resetmbr")
    return out 


def list_device():
    return send_request("device list")


def device_monitoring_state():
    return send_request("request monitoring_state")

def wbt_request(event_name, argument):
    return send_request("wbt " + event_name + " " + argument)


def stop_rebuilding(array):
    param_str = "--array-name " + array
    return send_request("internal stop_rebuilding " + param_str)


def create_uram(devname, block_size, num_blocks):
    param_str = "--device-type uram"
    param_str += " --device-name " + devname
    param_str += " --block-size " + block_size
    param_str += " --num-blocks " + num_blocks
    return send_request("device create " + param_str)
