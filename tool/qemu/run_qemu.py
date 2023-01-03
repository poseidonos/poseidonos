#!/usr/bin/env python3

import platform
from uuid import getnode
import re
import argparse
import subprocess
import sys
import os
import getpass
POS_ROOT = os.path.dirname(os.path.abspath(__file__)) + "/../.."

if not os.path.exists(f"{POS_ROOT}/tool/arion/lib"):
    subprocess.call("git submodule init", shell=True)
    subprocess.call("git submodule update", shell=True)
sys.path.append(POS_ROOT + "/tool/arion")
import lib

#################################################################
use_local_server = False
HOME_DIR = os.path.expanduser('~')
vm_path = f"{HOME_DIR}/virtual/qemu-ubuntu"
iso_path = "ubuntu-18.04.6-server-arm64.iso"   # Need to change the path to real location
virtual_image_name = f"pos_{platform.machine()}_ubuntu1804.qcow2"
virtual_image_path = ""
virtual_image_size = "128G"
virtual_device_path = ""
nvme_device_prefix = "nvme"
nvme_device_size = "20G"        # Required minimum size from POS
num_device = 5
print_log = True
#################################################################
num_core = 8
num_socket = 1
total_memory_gb_size = 14             # Required minimum size from POS
logical_block_size = 512
physical_block_size = 512
#################################################################


def install_qemu():
    if platform.system() == "Darwin":
        subprocess.call("brew install ninja pkgconfig glib pixman qemu", shell=True)
    elif platform.system() == "Linux":
        subprocess.call("sudo apt update", shell=True)
        subprocess.call("sudo apt install -y libvirt-clients libpixman-1-dev libglib2.0-dev qemu-kvm tigervnc-viewer sshpass uml-utilities ovmf python3-pip meson", shell=True)
        subprocess.call("sudo pip3 install ninja", shell=True)
        subprocess.call("git clone -b v7.1.0 https://github.com/qemu/qemu.git", shell=True)
        subprocess.call("cd qemu; ./configure --with-git-submodules=update", shell=True)
        subprocess.call("cd qemu; make -j ; make install", shell=True)


def make_qemu_image():
    if not os.path.exists(virtual_image_path):
        os.makedirs(virtual_image_path)
        lib.subproc.sync_run(
            f"qemu-img create -f qcow2 {virtual_image_path}/{virtual_image_name} {virtual_image_size}")
        qemu_command = ""
        if platform.system() == "Darwin" and platform.machine() == "arm64":
            if use_local_server:
                local_address = input("Please enter local service server address: ")
                password = getpass.getpass("Please enter server password: ")
                lib.printer.yellow(f"Downloading QEMU Image from local server. (Destination: {virtual_image_path}/{virtual_image_name})")
                subprocess.call(f"sshpass -p {password} scp -r root@{local_address}:/psdData/util/pos_aarch64_ubuntu_1804_server.qcow2 {virtual_image_path}/{virtual_image_name}", shell=True)
            else:
                if not os.path.exists(iso_path):
                    subprocess.call(f"eval curl -LO 'https://cdimage.ubuntu.com/releases/18.04/release/ubuntu-18.04.6-server-arm64.iso'", shell=True)
                    lib.subproc.sync_run(f"mv ubuntu-18.04.6-server-arm64.iso {iso_path}")
                qemu_command = "qemu-system-aarch64 "
                qemu_command += "-serial mon:stdio "
                qemu_command += "-accel hvf "
                qemu_command += "-cpu host "
                qemu_command += f"-smp {num_core} "
                qemu_command += f"-m {total_memory_gb_size}G "
                qemu_command += f"-bios QEMU_EFI.fd "
                qemu_command += "-device qemu-xhci "
                qemu_command += f"-drive file={virtual_image_path}/{virtual_image_name},if=virtio,cache=writethrough "
                qemu_command += f"-drive file={iso_path},id=cdrom,if=none,media=cdrom "
                qemu_command += "-device virtio-scsi-device -device scsi-cd,drive=cdrom "
                qemu_command += "-M virt"
        elif platform.machine() == "x86_64":
            if not os.path.exists(iso_path):
                subprocess.call(
                    f"eval curl -LO 'https://mirror.kakao.com/ubuntu-releases/18.04.6/ubuntu-18.04.6-live-server-amd64.iso'", shell=True)
                lib.subproc.sync_run(
                    f"mv ubuntu-18.04.6-live-server-amd64.iso {iso_path}")
            qemu_command = "qemu-system-x86_64 "
            qemu_command += "-serial mon:stdio "
            qemu_command += "-cpu host "
            qemu_command += f"-smp {num_core} "
            qemu_command += f"-m {total_memory_gb_size}G "
            if platform.system() == "Darwin":
                qemu_command += "-accel hvf "
                qemu_command += "-drive if=pflash,format=raw,readonly=yes,file=usr/share/OVMF/OVMF_CODE.fd "
                qemu_command += "-drive if=pflash,format=raw,file=usr/share/OVMF/OVMF_VARS.fd "
            else:
                qemu_command += "-enable-kvm "
                qemu_command += "-drive if=pflash,format=raw,readonly=yes,file=/usr/share/OVMF/OVMF_CODE.fd "
                qemu_command += "-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd "
            qemu_command += "-device qemu-xhci "
            qemu_command += f"-drive if=virtio,file={virtual_image_path}/{virtual_image_name},cache=writethrough "
            qemu_command += "-net user,hostfwd=tcp::60022-:22 "
            qemu_command += "-net nic,model=virtio "
            qemu_command += f"-cdrom {iso_path}"

        subprocess.call(qemu_command, shell=True)


def make_device_image():
    if not os.path.exists(virtual_device_path):
        os.makedirs(virtual_device_path)
        for index in range(num_device):
            lib.subproc.sync_run(f"qemu-img create {virtual_device_path}/{nvme_device_prefix}.{index}.raw {nvme_device_size}")


def start_qemu():
    host_mac_address = ':'.join(re.findall('..', '%012x' % getnode())[0:-1])
    host_mac_address += ":%0.2X" % args.id

    qemu_command = ""
    if platform.system() == "Darwin" and platform.machine() == "arm64":
        qemu_command = "qemu-system-aarch64 "
        qemu_command += "-accel hvf "
        qemu_command += "-bios QEMU_EFI.fd "
        qemu_command += f"-nic vde,model=virtio,sock=/var/run/vde.ctl,mac={host_mac_address} "
    elif platform.machine() == "x86_64":
        qemu_command = "qemu-system-x86_64 "
        if platform.system() == "Darwin":
            qemu_command += "-accel hvf "
            qemu_command += "-drive if=pflash,format=raw,readonly=yes,file=usr/share/OVMF/OVMF_CODE.fd "
            qemu_command += "-drive if=pflash,format=raw,file=usr/share/OVMF/OVMF_VARS.fd "
        elif platform.system() == "Linux":
            qemu_command += "-enable-kvm "
            qemu_command += "-drive if=pflash,format=raw,readonly=yes,file=/usr/share/OVMF/OVMF_CODE.fd "
            qemu_command += "-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd "
        qemu_command += "-net user,hostfwd=tcp::60022-:22 "
        qemu_command += f"-net nic,model=virtio,macaddr={host_mac_address} "
    qemu_command += "-cpu host "
    qemu_command += "-serial mon:stdio "
    qemu_command += "-msg timestamp=on "
    qemu_command += f"-smp {num_core},sockets={num_socket},cores={num_core//num_socket},maxcpus={num_core} "
    qemu_command += f"-m {total_memory_gb_size}G "
    numa_mem_gb_size = total_memory_gb_size / num_socket
    num_core_per_numa = num_core / num_socket
    for index in range(num_socket):
        qemu_command += f"-object memory-backend-ram,id=ram-node{index},size={numa_mem_gb_size:.0f}G "
        qemu_command += f"-numa node,memdev=ram-node{index},cpus={index * num_core_per_numa:.0f}-{index + num_core_per_numa - 1:.0f},nodeid={index} "

    qemu_command += f"-drive file={virtual_image_path}/{virtual_image_name},if=virtio,cache=writethrough "
    qemu_command += "-object rng-random,id=objrng0,filename=/dev/urandom "
    qemu_command += "-device qemu-xhci "
    for index in range(0, num_device):
        qemu_command += f"-drive file={virtual_device_path}/{nvme_device_prefix}.{index}.raw,if=none,id=nvme-ctrl-{nvme_device_prefix}{index},format=raw -device nvme,serial=pos{nvme_device_prefix}{index},drive=nvme-ctrl-{nvme_device_prefix}{index} "

    if platform.system() == "Darwin" and platform.machine() == "arm64":
        qemu_command += "-M virt"
    elif platform.system() == "Linux" and platform.machine() == "x86_64":
        qemu_command = "sudo " + qemu_command

    subprocess.call(qemu_command, shell=True)


def check_platform():
    if not ((platform.system() == "Darwin" and platform.machine() == "arm64") or (platform.system() == "Darwin" and platform.machine() == "x86_64") or (platform.system() == "Linux" and platform.machine() == "x86_64")):
        lib.printer.red(f"Not support platform. Current OS: {platform.system()}, CPU: {platform.machine()}")
        exit()


def prepare_precondition(args):
    check_platform()
    if platform.system() == "Darwin" and platform.machine() == "x86_64":
        if not os.path.exists(f"{POS_ROOT}/tool/qemu/usr"):
            subprocess.call("brew install rpm", shell=True)
            lib.subproc.sync_run("rpm2cpio < edk2-ovmf-20210527gite1999b264f1f-2.fc35.noarch.rpm | cpio -id")

    global vm_path
    global virtual_device_path
    global virtual_image_path
    vm_path = vm_path + "/" + "vm" + str(args.id)
    virtual_device_path = vm_path + "/devices"
    virtual_image_path = vm_path + "/image"

    if not os.path.exists(vm_path):
        os.makedirs(vm_path)
        lib.printer.green(f"The new directory is created. ({vm_path})")

    lib.subproc.set_print_log(print_log)


def parse_arguments(args):
    parser = argparse.ArgumentParser(description='code formatter')
    parser.add_argument('-q', '--need_install_qemu', required=False, default=False,
                        action='store_true', help='Enable the option to install QEMU on this system')
    parser.add_argument('-m', '--need_make_qemu_image', default=False, required=False,
                        action='store_true', help='Enable the option to create the ubuntu virtual server using QEMU')
    parser.add_argument('-d', '--need_create_nvme_device', default=False, required=False,
                        action='store_true', help='Enable the option to create the virtual nvme block device')
    parser.add_argument('-i', '--id', default='',
                        required=True, type=int, help='VM id')
    args = parser.parse_args()

    return args


if __name__ == "__main__":
    args = parse_arguments(sys.argv)

    prepare_precondition(args)
    if (args.need_install_qemu is True):
        install_qemu()
    if (args.need_make_qemu_image is True):
        make_qemu_image()
    if (args.need_create_nvme_device is True):
        make_device_image()

    start_qemu()
