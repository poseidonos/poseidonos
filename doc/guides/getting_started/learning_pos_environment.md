# Overview
POS relies on external software components to implement its functionalities. It leverages SPDK library to satisfy NVM subsystem semantics, communicates with Linux kernel for PCI and virtual memory management, and offers web UI by using nginx. In this section, we will cover a list of environment configs that could affect the behavior of POS. 

## Table of Contents
- [Linux Kernel](#linux-kernel)
  - [udev rule](#udev-rule)
  - [hugepages](#hugepages)
  - [max virtual memory map count](#max-virtual-memory-map-count)
  - [tcp parameters](#tcp-parameters)
- [SPDK Resources](#spdk-resources)
  - [bdev](#bdev)
  - [NVM Subsystem](#nvm-subsystem)
  - [NVMe-oF transport](#nvme-of-transport)
  - [NVMe-oF listener](#nvme-of-listener)
- [Management Dashboard](#management-dashboard)


## Linux Kernel
### udev rule
* defines a rule to follow when Linux detects hotplug events in subsystems, 
  - e.g., PCI attach/detach.
* The default rule by POS is to detach newly-discovered NVMe devices from Linux kernel driver ("nvme") and attach to user level driver ("uio_pci_generic"). You can auto configure udev rule by executing following command:   
```
cd $POS_HOME; make udev_install
```  
If needed, you can customize the rule further at ```/etc/udev/rules.d/99-custom-nvme.rules```. 

### hugepages
* are reserved and kept from being swapped out so that memory paging in/out operations
* are removed and hit ratio in CPU can be also increased.
* are mainly used for SPDK library which supports user level device driver, so POS also needs to reserve them to use SPDK library.
  - This reservation can be done by running ```NRHUGE=XXXXX $POS_HOME/lib/$SPDK/scripts/setup.sh```
* becomes available to Linux only when "hugetlbfs" is mounted on ```/dev/hugepages```
  - e.g., hugetlbfs on ```/dev/hugepages``` type hugetlbfs (rw,relatime,pagesize=2M) should appear in the output of mount command. 
* becomes available to application (i.e., POS) only when the required amount is specified through proc fs.
  - e.g., ```echo $NRHUGE > /proc/sys/vm/nr_hugepages```
* currently, ```$NRHUGE``` is calculated as 2 / 3 of the total memory in the unit of 2 MB pages. If needed, this can be adjusted in ```$POS_HOME/script/setup_env.sh```
  - e.g., ```$(cat /proc/meminfo | grep MemTotal | awk '{print $2}') / 3 * 2 / 2 / 1024```

### max virtual memory map count
* is set to Math.min(65536, current_max_map_count) where current_max_map_count is from ```/proc/sys/vm/max_map_count```
* If you observe an issue with mmap()/mprotect() in POS, please consider increasing it to higher value, 
  - e.g., 262144, in ```$POS_HOME/script/setup_env.sh```

### tcp parameters
* can be optimized on the target side if needed.
* POS offers an example script as in the following and the numbers are from experiments.
  - ```$POS_HOME/test/system/network/tcp_tune.sh max```

```bash
sysctl -w net.core.rmem_default="268435456"
sysctl -w net.core.rmem_max="268435456"
sysctl -w net.core.wmem_default="268435456"
sysctl -w net.core.wmem_max="268435456"
sysctl -w net.ipv4.tcp_wmem="4096 65536 134217728"
sysctl -w net.ipv4.tcp_rmem="4096 131072 134217728"
sysctl -w net.ipv4.tcp_mtu_probing="1"
sysctl -w net.ipv4.tcp_window_scaling="1"
sysctl -w net.ipv4.tcp_slow_start_after_idle="0"
```

## SPDK Resources
### bdev
* is a SPDK-provided block device that runs at user level.
* can be configured by a RPC call to SPDK server that gets spawned during POS initialization and listens on localhost:5260
  - ```$POS_HOME/lib/spdk-20.10/scripts/rpc.py bdev_malloc_create  {total_size } {block_size }```
  - *(mandatory)* ```total size```: the size of bdev in MB
  - *(mandatory)* ```block_size```: the size of a block in the bdev
  - *(optional)* ```-b```: the name of the block device name to use. If the name starts with "uram", SPDK library automatically populates the metadata of the allocated hugepage at /tmp/uram_hugepage, which includes pid, startpage, and pageCount. If the option is omitted, SPDK library appends monotonically-increasing non-negative integer to "Malloc". The device name would look like Malloc0, Malloc1, Malloc2, ..., and so in subsequent runs. 
* ```bdev_malloc``` is a block device which is created as a user space ramdisk used for write buffer.
* The recommended total_size and block_size for a PSD server is 8192 and 512 for now.

### NVM Subsystem
* presents a collection of controller(s) which are used to access NVM namespaces.
* can be configured by a RPC call
  - ```$POS_HOME/lib/spdk-20.10/scripts/rpc.py nvmf_create_subsystem  {nqn }```
  - *(mandatory)* ```nqn```: Nvm Qualified Name that uniquely identifies NVMe-oF target
  - *(mandatory)* ```-s```: the serial number of the target. Any vendor-identifiable string should be okay.
  - *(mandatory)* ```-d```: the model number of the target. Any vendor-identifiable string should be okay.
  - *(mandatory)* ```-m```: the maximum number of namespaces of the target.
  - *(optional)* ```-t```: the type of the target. It is "nvmf_tgt" by default.
* The maximum number of namespaces (```-m```) determines the maximum number of POS volumes we could have for an NVM subsystem. Currently, max 256 POS volumes are supported.
* Each namespace for a controller is mapped to POS volume.


### NVMe-oF transport
* determines the data transfer mechanism between an initiator and a target
* can be configured by a RPC call
  - ```$POS_HOME/lib/spdk-20.10/scripts/rpc.py nvmf_create_transport```
  - *(mandatory)* ```-t```: the type of the transport. It should be either "tcp" or "rdma". 
    - Please make sure the POS binary is built with ```-DSPDK_CONFIG_RDMA``` to be able to use RDMA.
  - *(mandatory)* ```-b```: the number of shared buffers to reserve for each poll group
  - *(mandatory)* ```-n```: the number of pooled data buffers available to the transport
  - *(optional)* ```-g```: the type of the target. "nvmf_tgt" by default.
  - *(optional)* ```-q```: the maximum number of outstanding I/O per queue. 128 by default.
  - *(optional)* ```-p```: the maximum number of SQ and CQ per controller. 128 by default.
  - *(optional)* ```-c```: the maximum number of in-capsule data size. 4096 by default.
  - *(optional)* ```-u```: the size of I/O in bytes. 131072 by default.
  - *(optional)* ```-a```: the maximum number of admin commands per AQ. 128 by default
  - *(optional)* ```-s```: the maximum number of outstanding I/Os per SRQ. Relevant only for RDMA. 4096 by default.
  - *(optional)* ```-r```: true to disable per-thread shared receive queue and false to enable it. Relevant only for RDMA. false by default.
  - *(optional)* ```-o```: true to disable C2H success optimization and false to enable it. Relevant only for TCP. true by default.
  - *(optional)* ```-f```: true to enable DIF insert/strip and false to disable it. Relevant only for TCP. false by default.
  - *(optional)* ```-y```: the sock priority of the tcp connection. Relevant only for TCP. 0 by default.


### NVMe-oF listener
* determines transport endpoint configuration.
* can be configured by a RPC call
  - ```$POS_HOME/lib/spdk-20.10/scripts/rpc.py nvmf_subsystem_add_listener {nqn}```
  - *(mandatory)* ```nqn```: Nvm Qualified Name that uniquely identifies NVMe-oF target
  - *(mandatory)* ```-t```: the type of the transport. It should be either "tcp" or "rdma".
  - *(mandatory)* ```-a```: the transport address to listen on,
    - e.g., IP address
  - *(mandatory)* ```-s```: the transport port to listen on,
    - e.g., TCP port
  - *(optional)* ```-p```: the name of the parent NVMe-oF target. nvmf_tgt by default.
  - *(optional)* ```-f```: the name of the NVMe-oF transport address family. It should be ipv4, ipv6, ib, or fc (case-insensitive).


## Management Dashboard

The web dashboard endpoint can be configured by modifying nginx configuration and restarting the service:

```bash
$ sudo vi /etc/nginx/conf.d/virtual.conf
$ sudo service nginx reload
```

The default virtual.conf looks like the following. This configuration says the dashboard is available at (10.1.11.20, 80). Please refer to Learning POS Management Tool for more details. 

```
upstream mtool {
     server    10.1.11.20:5000 weight=50;
}
upstream dagent {
     server    10.1.11.20:3000 weight=50;
}
server {
     listen    80;
     server_name    10.1.11.20;
     proxy_read_timeout    50000;
location / {
           proxy_pass http://mtool;
    }
     location /api/dagent {
           proxy_pass http://dagent;
    }
     location /api/ibofos {
           proxy_pass http://dagent;
    }
     location /api/metric {
           proxy_pass http://dagent;
    }
     location /redfish {
           proxy_pass http://dagent;
    }
     location ^~ /api/v1.0 {
           proxy_pass http://mtool;
    }
     location /redfish/v1/StorageServices {
           proxy_pass http://mtool;
    }
}
```
