Poseidon Quick EMUlator(QEMU) Script
=====
QEMU/KVM offers powerful emulating features, including the ability to create many kinds of NVMe block device for your VMs. These can also provide a Non-Uniform Memory Access (NUMA) environment based on Xeon Processor. However, a common problem with QEMU/KVM is how it deals with various options.

PoseidonOS has some server requirements to run (several NVMe devices, NUMA etc.,). Since it is not easy to setup the environment in a general PC. So we're gonna to provide a script that can make setup virtual server easily.

## Environments
In this lists, the following environments are checked:

|  |  Set 1  |  Set 2  |
| ------ | ------- | ------- |
| CPU  | M1 Pro | Intel i7-9700 |
| OS  | MacOS 12.5 | Windows 11 + WSL Ubuntu |

## Requirements
- qemu
- enabled virtualization (KVM)
- [Ubuntu 18.04 iso](https://releases.ubuntu.com/18.04/ubuntu-18.04.6-live-server-amd64.iso)
- [vde_vmnet](https://github.com/lima-vm/vde_vmnet) >= v0.6.0: Only for Mac. 
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

## How to Install vde_vmnet (for Mac)
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
