# Factory Reset
You may want to clean up existing states and start POS from scratch. One simple way is to run POS-provided script called "$POS_HOME/script/factory_reset.py", which executes the following steps:

Send SIGKILL to ibofos (i.e., POS process) if it is running
Initialize POS config by copying a file from $POS_HOME/config/pos.conf to /etc/pos/pos.conf
Delete POS log files matching /var/log/pos/*.log
Reset MBR partition by invoking $POS_HOME/test/script/mbr_reset.sh
Reset Udev Rule file by invoking $POS_HOME/tool/udev/reset_udev_rule.sh
Reset device drivers by invoking $POS_HOME/script/setup_env.sh
99-custom-nvme.rules looks like the following, though the actual content may differ by environments:

```bash
ACTION=="bind", KERNELS=="0000:04:00.0" , SUBSYSTEMS=="pci", DRIVERS=="nvme", ATTRS{class}=="0x010802" , \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/nvme/unbind'", \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/uio_pci_generic/bind'"
 
 
ACTION=="bind", KERNELS=="0000:0c:00.0" , SUBSYSTEMS=="pci", DRIVERS=="nvme", ATTRS{class}=="0x010802" , \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/nvme/unbind'", \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/uio_pci_generic/bind'"
 
 
ACTION=="bind", KERNELS=="0000:13:00.0" , SUBSYSTEMS=="pci", DRIVERS=="nvme", ATTRS{class}=="0x010802" , \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/nvme/unbind'", \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/uio_pci_generic/bind'"
 
 
ACTION=="bind", KERNELS=="0000:1b:00.0" , SUBSYSTEMS=="pci", DRIVERS=="nvme", ATTRS{class}=="0x010802" , \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/nvme/unbind'", \
RUN+="/bin/bash -c 'echo $kernel > /sys/bus/pci/drivers/uio_pci_generic/bind'"
```