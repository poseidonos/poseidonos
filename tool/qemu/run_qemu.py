#!/usr/bin/env python3

import os
import sys
import subprocess
import argparse
import re
from uuid import getnode

iso_path = "/mnt/d/Download/ubuntu-18.04.6-live-server-amd64.iso"
vm_img = "pos-ubuntu1804.qcow2"
qemu_dir = "/mnt/d/Virtual/qemu-ubuntu"
device_dir = "devices"
image_dir = "images"
vm_prefix = "vm"
nvme_device_prefix = "nvme"
num_device = 5

###########################
logical_block_size = 512
physical_block_size = 512
###########################


def install_qemu():
    subprocess.call(f"./install_qemu.sh", shell=True)


def make_qemu_image(vm_dir):
    dest_dir = vm_dir + "/" + image_dir
    subprocess.call(
        f"./create_qemu_image.sh {dest_dir} {vm_img} {iso_path}", shell=True)


def make_device_image(vm_dir):
    dest_dir = vm_dir + "/" + device_dir
    subprocess.call(f"./create_device.sh {dest_dir} 4", shell=True)


def start_qemu(vm_dir):
    dest_device = vm_dir + "/" + device_dir
    dest_image = vm_dir + "/" + image_dir

    host_mac_address = ':'.join(re.findall('..', '%012x' % getnode())[0:-1])
    host_mac_address += ":%0.2X" % args.id
    qemu_command = f"qemu-system-x86_64 "
    qemu_command += "-m 24G "
    qemu_command += "-enable-kvm "
    qemu_command += "-drive if=pflash,format=raw,readonly=yes,file=/usr/share/OVMF/OVMF_CODE.fd "
    qemu_command += "-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd "
    qemu_command += f"-drive if=virtio,file={dest_image}/{vm_img},cache=none "
    qemu_command += "-smp 8,sockets=2,cores=4,maxcpus=8 "
    qemu_command += "-object memory-backend-ram,id=ram-node0,size=12G "
    qemu_command += "-object memory-backend-ram,id=ram-node1,size=12G "
    qemu_command += "-numa node,memdev=ram-node0,cpus=0-3,nodeid=0 "
    qemu_command += "-numa node,memdev=ram-node1,cpus=4-7,nodeid=1 "
    qemu_command += "-cpu Skylake-Server "
    qemu_command += "-rtc base=utc,driftfix=slew "
    qemu_command += "-global ICH9-LPC.disable_s3=1 "
    qemu_command += "-global ICH9-LPC.disable_s4=1 "
    qemu_command += "-boot strict=on "
    qemu_command += "-machine q35,accel=kvm,kernel-irqchip=split "
    qemu_command += "-object rng-random,id=objrng0,filename=/dev/urandom "
    qemu_command += "-msg timestamp=on "
    qemu_command += "-device intel-iommu "
    qemu_command += "-net user,hostfwd=tcp::2222-:22 "
    qemu_command += f"-net nic -device virtio-net "
    qemu_command += "-device pcie-root-port,port=0x10,chassis=1,id=pci.1,bus=pcie.0,multifunction=on,addr=0x4 "

    for index in range(0, num_device):
        qemu_command += f"-device nvme,id=nvme-ctrl-{nvme_device_prefix}{index},serial=pos{nvme_device_prefix}{index} "
        qemu_command += f"-drive file={dest_device}/{nvme_device_prefix}.{index}.raw,format=raw,if=none,id={nvme_device_prefix}-{index}1 "
        qemu_command += f"-device nvme-ns,drive={nvme_device_prefix}-{index}1,logical_block_size={logical_block_size},physical_block_size={physical_block_size} "

    subprocess.call(qemu_command, shell=True)


def parse_arguments(args):
    parser = argparse.ArgumentParser(description='code formatter')
    parser.add_argument('-q', '--need_install_qemu', default=False, type=bool, help='Enable the option to install QWEMU on this system')
    parser.add_argument('-m', '--need_make_qemu_image', default=False, type=bool, help='Enable the option to create the ubuntu virtual server using QEMU')
    parser.add_argument('-c', '--need_create_nvme_device', default=False, type=bool, help='Enable the option to create the virtual nvme block device')
    parser.add_argument('-i', '--id', default='', type=int, help='VM id')
    args = parser.parse_args()

    return args


def print_help():
    print("Usage: run_qemu.py -i <vm_index>")


if __name__ == "__main__":
    args = parse_arguments(sys.argv)
    if args.id == '':
        print_help()
        exit
    else:
        vm_prefix += str(args.id)
        if(args.need_install_qemu is True):
            install_qemu()
        if(args.need_make_qemu_image is True):
            make_qemu_image((qemu_dir + "/" + vm_prefix))
        if(args.need_create_nvme_device is True):
            make_device_image((qemu_dir + "/" + vm_prefix))

    start_qemu((qemu_dir + "/" + vm_prefix))
