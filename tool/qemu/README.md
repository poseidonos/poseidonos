Poseidon Quick EMUlator(QEMU) script
=====
QEMU/KVM offers powerful emulating features, including the ability to create many kinds of NVMe block device for your VMs. These can also provide a Non-Uniform Memory Access (NUMA) environment based on Xeon Processor. However, a common problem with QEMU/KVM is how it deals with various options.

PoseidonOS has some server requirements to run (several NVMe devices, NUMA etc.,). Since it is not easy to setup the environment in a general PC. So we're gonna to provide a script that can make setup virtual server easily.

## Requirements
- qemu-system-x86_64
- enabled virtualization (KVM)
- [Ubuntu 18.04 iso](https://releases.ubuntu.com/18.04/ubuntu-18.04.6-live-server-amd64.iso)
## How to Run script?
Run QEMU script
```bash
./run_qemu.py -q False -m False -c False -i 1
```

## Usage Notes
- `-i`: (Mandatory) Set the index of virtual machine

- `-q`: (Optional) Enable the option to install QEMU on this system

- `-m`: (Optional) Enable the option to create the ubuntu virtual server using QEMU

- `-c`: (Optional) Enable the option to create the virtual nvme block device