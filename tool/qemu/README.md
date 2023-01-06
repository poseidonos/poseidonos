Poseidon Quick EMUlator(QEMU) Script
=====
PoseidonOS has a few requirements to be able to function properly, such as the minimum number of NVMe devices and the NUMA affinity setting, which used to make it hard to run in certain user environments.

QEMU/KVM offers powerful emulating features, including the ability to create many kinds of NVMe block device for your VMs. These can also provide Non-Uniform Memory Access (NUMA) environment. However, a common problem with QEMU/KVM is in the complexity of dealing with various options.

So, we are going to provide a script that helps to easily set up a virtual server for PoseidonOS to run on various environments.

## Environments
We have confirmed that the script works for the environments in the following list:

|  |  Set 1  |  Set 2  | Set 3 |
| ------ | ------- | ------- | ------- |
| CPU  | M1 Pro | Intel i7-9700 | AMD ryzen 5600x |
| OS  | MacOS 12.5 | Windows 11 + WSL Ubuntu | Windows 11 + WSL Ubuntu |

## Requirements
- QEMU 7.1.0 (We checked only this version)
- enabled virtualization (KVM)
- [Ubuntu 18.04 iso](https://releases.ubuntu.com/18.04/ubuntu-18.04.6-live-server-amd64.iso)
- [vde_vmnet](https://github.com/lima-vm/vde_vmnet) >= v0.6.0: This is a framework to use a virtual network accessible by a host on Apple Silicon Mac. Here is the [guide](#how-to-install-vde_vmnet-only-for-apple-silicon-mac) to install.
- edk2-ovmf: OVMF(Open Virtual Machine Firmware) is required to enable UEFI support for Intel MacOS.
- 128GB + 5 * 20GB stoage is required minimum space for virtual OS disk and virtual nvme devices. But you can configurate these sizes.
- 14GB DRAM is required minimum size for PoseidonOS.

## How to Run Script?
Run QEMU script
```bash
./run_qemu.py -i 1 -q -m -d
```

## Usage Notes
- `-i`: (Mandatory) Set the index of virtual machine

- `-q`: (Optional) Enable the option to install QEMU on this system

- `-m`: (Optional) Enable the option to create the ubuntu virtual server using QEMU

- `-d`: (Optional) Enable the option to create the virtual nvme block device

## How to Install vde_vmnet (only for Apple Silicon Mac)
```bash
# Install vde_vmnet.
brew install automake autoconf libtool
git clone https://github.com/lima-vm/vde_vmnet
cd vde_vmnet
git config --global --add safe.directory "$(pwd)"
sudo make PREFIX=/opt/vde install

# Unload
sudo launchctl unload -w "/Library/LaunchDaemons/io.github.lima-vm.vde_vmnet.plist"
sudo launchctl unload -w "/Library/LaunchDaemons/io.github.virtualsquare.vde-2.vde_switch.plist"

# Load (order matters)
sudo launchctl load -w "/Library/LaunchDaemons/io.github.virtualsquare.vde-2.vde_switch.plist"
sudo launchctl load -w "/Library/LaunchDaemons/io.github.lima-vm.vde_vmnet.plist"
```
