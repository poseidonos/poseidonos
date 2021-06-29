#!/usr/bin/env python3

import argparse
import nvmf_common
import os
import subprocess

POS_ROOT_DIR = os.path.dirname(os.path.abspath(__file__)) + "/../../.."
SPDK_DIR = POS_ROOT_DIR + "/lib/spdk"
SPDK_RPC = SPDK_DIR + "/scripts/rpc.py"

#sudo $SPDK_DIR/scripts/rpc.py nvmf_get_subsystems

############## DEFAULT VALUES START ################
DEFAULT_RAID_BDEV_NAME = "spdk_raid0"
DEFAULT_NVMF_TRANSPORT_TYPE = "tcp"
DEFAULT_SHARED_BUFFER_COUNT_PER_TRANSPORT=2048
DEFAULT_SHARED_BUFFER_COUNT_PER_POLL_GROUP=64
DEFAULT_IO_UNIT_SIZE_IN_BYTES_FOR_TRANSPORT=131072
DEFAULT_SUBSYSTEM_NQN = "nqn.2019-04.spdk-raid:subsystem1"
DEFAULT_TARGET_TRANSPORT_ADDRESS = "172.16.1.1"
DEFAULT_TARGET_TRANSPORT_SERVICE_ID = "1158"
DEFAULT_SERIAL_NUMBER = "DEFAULT SERIAL"
DEFAULT_MODEL_NUMBER = "DEFAULT MODEL"
DEFAULT_RAID_LEVEL = "0"

############## DEFAULT VALUES END ##################

def scan_unvme_controllers():
    command = SPDK_DIR + "/scripts/setup.sh status | grep uio_pci_generic " \
        + "| grep - | gawk '{ print $1 }' "
    bdf_output = subprocess.check_output(command,
                        shell=True, universal_newlines=True)
    bdf_output = bdf_output[:-1]

    controller_prefix = "unvme"
    bdf_list = bdf_output.split('\n')

    controller_dict = dict()
    controller_index = 0

    for bdf_addr in bdf_list:
        controller_dict[controller_prefix + str(controller_index)] = bdf_addr
        controller_index += 1

    return controller_dict    


def attach_controller(bdev_name, transport, bdf_addr):
    command = SPDK_RPC + " bdev_nvme_attach_controller "
    command += "-b " + bdev_name + " "
    command += "-t " + transport + " "
    command += "-a " + bdf_addr + " "

    allocated_bdev_name = subprocess.check_output(command, shell=True,
                                universal_newlines=True)

    return allocated_bdev_name.replace('\n', '')

def create_raid_bdev(bdev_name, raid_level, stripe_size_in_kb,
        bdev_list_to_tie_up):
    command = SPDK_RPC + " bdev_raid_create "
    command += "-n " + bdev_name + " "
    command += "-r " + str(raid_level) + " "
    command += "-z " + str(stripe_size_in_kb) + " "
    command += "-b '"

    for bdev in bdev_list_to_tie_up:
        command += bdev + " "

    command += "'"

    print(command)
    returnCode = subprocess.call(command, shell=True)
    return (0 == returnCode)

def create_nvmf_transport(transport_type,
        shared_buffer_count_per_poll_group=None,
        shared_buffer_count_per_transport=None,
        io_unit_size_in_bytes=None):

    expected_result = True
    nvmf = nvmf_common.Nvmf("[ Create Transport ", expected_result, SPDK_RPC)
    successful = nvmf.create_transport(transport_type,
        shared_buffer_count_per_poll_group,
        shared_buffer_count_per_transport, io_unit_size_in_bytes)

    return successful

def create_nvmf_subsystem(subnqn, allow_any_host=False,
        serial_number="DEFAULT SERIAL",
        model_number="DEFAULT MODEL",
        max_namespace_count=1):

    expected_result = True
    nvmf = nvmf_common.Nvmf("[ Create Subsystem ", expected_result, SPDK_RPC)
    successful = nvmf.create_subsystem(subnqn, serial_number, model_number,
            max_namespace_count, allow_any_host)
    
    return successful

def add_listener_to_nvmf_subsystem(subnqn,
        transport_type, transport_address, transport_service_id):

    expected_result = True
    nvmf = nvmf_common.Nvmf("[ Add Subsystem Listener ",
                expected_result, SPDK_RPC)
    successful = nvmf.add_subsystem_listener(subnqn,
            transport_type, transport_address, transport_service_id)

    return successful


def add_namespace_to_nvmf_subsystem(subnqn, bdev_name, namespace_id=0):
    expected_result = True
    nvmf = nvmf_common.Nvmf("[ Add namespace to Subsystem ", expected_result,
                SPDK_RPC)
    successful = nvmf.add_namespace_to_subsystem(subnqn, bdev_name,
                    namespace_id)

    return successful


def parse_arguments():
    parser = argparse.ArgumentParser(description='Please input parameter(s)'\
                ' needed')

    parser.add_argument('--trtype', required=False,
        help='Transport type for NVMe-oF [rdma|tcp]',
        default=DEFAULT_NVMF_TRANSPORT_TYPE)
    parser.add_argument('--shbuftr', required=False,
        help='Number of shared buffers available to the transport',
        default=DEFAULT_SHARED_BUFFER_COUNT_PER_TRANSPORT)
    parser.add_argument('--shbufpg', required=False,
        help='Number of shared buffers to reserve for each poll group',
        default=DEFAULT_SHARED_BUFFER_COUNT_PER_POLL_GROUP)
    parser.add_argument('--iounitsize', required=False,
        help='I/O Unit size in bytes',
        default=DEFAULT_IO_UNIT_SIZE_IN_BYTES_FOR_TRANSPORT)
    parser.add_argument('--traddr', required=False,
        help='IP addresss for NVMe-oF target subsystem',
        default=DEFAULT_TARGET_TRANSPORT_ADDRESS)
    parser.add_argument('--trsvid', required=False,
        help='Transport service id for NVMe-oF device',
        default=DEFAULT_TARGET_TRANSPORT_SERVICE_ID)
    parser.add_argument('--subnqn', required=False,
        help='Subsystem NQN for NVMe-oF device',
        default=DEFAULT_SUBSYSTEM_NQN)
    parser.add_argument('--serial', required=False,
        help='Serial number for NVMe-oF device',
        default=DEFAULT_SERIAL_NUMBER)
    parser.add_argument('--model', required=False,
        help='Model number for NVMe-oF device',
        default=DEFAULT_MODEL_NUMBER)
    parser.add_argument('--bdevname', required=False,
        help='Name for RAID Bdev',
        default=DEFAULT_RAID_BDEV_NAME,
        dest='raid_bdev_name')
    parser.add_argument('--raidlevel', required=False,
        help='Level of RAID',
        default=DEFAULT_RAID_LEVEL,
        dest='raid_level')

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_arguments()

    bdf_addr = scan_unvme_controllers()
    print(bdf_addr)

    controllers = bdf_addr.keys()
    controller_bdevs = list()

    for controller in controllers:
        allocated_bdev_name = attach_controller(controller,
                                    "pcie", bdf_addr[controller])
        controller_bdevs.append(allocated_bdev_name)

    print("Scanned nvme controllers:")
    print(controller_bdevs)

    successful = create_raid_bdev(args.raid_bdev_name, raid_level=0,
                    stripe_size_in_kb=1024,
                    bdev_list_to_tie_up=controller_bdevs)
    if True == successful:
        print("Creating RAID0 bdev (" + args.raid_bdev_name + ") Succeeded:")
        print(controller_bdevs)

    successful = create_nvmf_transport(args.trtype, args.shbufpg, args.shbuftr,
        args.iounitsize)
    if True == successful:
        print("Creating " + args.trtype + " transport Succeeded.")

    successful = create_nvmf_subsystem(args.subnqn, allow_any_host=True,
        serial_number=args.serial,
        model_number=args.model,
        max_namespace_count=256)
    if True == successful:
        print("Creating nvmf subsystem suceeded.")

    successful = add_namespace_to_nvmf_subsystem(args.subnqn,
        args.raid_bdev_name, namespace_id=1)
    if True == successful:
        print("Adding namespace suceeded.")

    successful = add_listener_to_nvmf_subsystem(args.subnqn, args.trtype,
        args.traddr, args.trsvid)
    if True == successful:
        print("Adding listener succeeded.")

