#!/usr/bin/env python3

import subprocess
import pos
import sys

POS_ROOT = '../../../'
POS_CLI = POS_ROOT + "bin/poseidonos-cli"


def send_request(msg):
    print (msg)
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
        if raid == "NONE":
            param_str += " --no-raid"
        else:
            param_str += " --raid " + raid
    return send_request("array create " + param_str)


def auto_create_array(buffer, num_data, num_spare, array, raid):
    param_str = ""
    if (buffer != ""):
        param_str += "-b " + buffer
    param_str += " -d " + str(num_data)
    param_str += " -s " + str(num_spare)
    param_str += " --array-name " + array
    if raid != "":
        if raid == "NONE":
            param_str += " --no-raid"
        else:
            param_str += " --raid " + raid
    return send_request("array autocreate " + param_str)


def delete_array(array):
    param_str = ""
    param_str += "--array-name " + array
    return send_request("array delete " + param_str + " --force")


def mount_array(array):
    param_str = ""
    param_str += "--array-name " + array
    if pos.is_wt_enabled() is True:
        param_str += " -w"
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


def mount_volume(vol_name, array, subnqn, transport_type="", target_addr="", transport_service_id=""):
    param_str = "--volume-name " + vol_name
    param_str += " --array-name " + array
    if subnqn != "":
        param_str += " --subnqn " + subnqn + " --force"
    if transport_type != "":
        param_str += " -t " + transport_type
    if target_addr != "":
        param_str += " -i " + target_addr
    if transport_service_id != "":
        param_str += " -p " + transport_service_id
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
    return send_request("devel stop-rebuilding " + param_str)


def create_uram(devname, block_size, num_blocks, numa=0):
    param_str = "--device-type uram"
    param_str += " --device-name " + devname
    param_str += " --block-size " + block_size
    param_str += " --num-blocks " + num_blocks
    param_str += " --numa " + str(numa)

    return send_request("device create " + param_str)


def add_listener(subnqn, transport_type, target_addr, transport_service_id):
    param_str = "--subnqn " + subnqn
    param_str += " --trtype " + transport_type
    param_str += " --traddr " + target_addr
    param_str += " --trsvcid " + transport_service_id
    return send_request("subsystem add-listener " + param_str)

def create_transport(transport_type, buf_cache_size, num_shared_buf):
    param_str = " --trtype " + transport_type
    param_str += " --buf-cache-size " + buf_cache_size
    param_str += " --num-shared-buf " + num_shared_buf
    return send_request("subsystem create-transport " + param_str)

def apply_log_filter():
    return send_request("logger apply-filter")


def get_log_level():
    return send_request("logger get-level")


def logger_info():
    return send_request("logger info")


def create_subsystem(subnqn, serial, model, max_namespace):
    param_str = "--subnqn " + subnqn
    param_str += " --serial-number " + serial
    param_str += " --model-number " + model
    param_str += " --max-namespaces " + str(max_namespace)
    param_str += " -o"
    return send_request("subsystem create " + param_str)


def list_subsystem():
    return send_request("subsystem list")


def delete_subsystem(subnqn):
    param_str = "--subnqn " + subnqn
    return send_request("subsystem delete " + param_str + " --force")


def list_qos_policies(array, volume):
    param_str = ""
    param_str += " --volume-name " + volume
    param_str += " --array-name " + array
    return send_request("qos list " + param_str)


def reset_qos_policies(array, volume):
    param_str = ""
    param_str += " --volume-name " + volume
    param_str += " --array-name " + array
    return send_request("qos reset " + param_str)


def reset_event_wrr_policy():
    return send_request("devel reset-event-wrr")


def update_event_wrr_policy(policy_name, weight):
    param_str = ""
    param_str += " --name " + policy_name
    param_str += " --weight " + str(weight)
    return send_request("devel update-event-wrr " + param_str)


def smart(device):
    param_str = "--device-name " + device
    return send_request("device smart-log " + param_str)


def start_telemetry():
    return send_request("telemetry start ")


def stop_telemetry():
    return send_request("telemetry stop ")
