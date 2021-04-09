
When a new NVMe device is detected by Linux PCI subsystem, "nvme" device driver in the kernel is automatically attached to the device. Then, Linux udev framework uses POS-provided custom rule to unbind the device from the kernel and bind to a user-level device driver called "uio_pci_generic". At this point, POS is able to scan the device and use it. If there have been multiple plugs/unplugs for the same NVMe device, POS may skip "device scan" command to be able to access the device. This section describes how to test this feature manually.

Prerequisite
========
1. Check if hotplug configuration is enabled in your BIOS
2. Ensure that Linux kernel version is 2.4 or higher

Setup Udev Rule
========
In the root directory of POS, execute following command

```
root@ibofos-target:IBOF_HOME# cd ./tool/udev
root@ibofos-target:IBOF_HOME/tool/udev# ./reset_udev_rule.sh

0000:04:00.0 : Added to Udev Rule File
0000:0c:00.0 : Added to Udev Rule File
0000:13:00.0 : Added to Udev Rule File
0000:1b:00.0 : Added to Udev Rule File
```

You can now see the rule file 99-custom-nvme.rules in /etc/udev/rules.d/

Hotplug Test
========

1. Run ibofos
```
./script/start_ibofos.sh
```

2. Device scan
```
./bin/cli device scan
```

3. Check device list
```
./bin/cli device list
```

4. Detach one pci device
```
echo 1 > /sys/bus/pci/devices/${bdf}/remove
```

5. Check device removed
```
./bin/cli device list
```

6. Re-attach the device with PCI rescan
```
echo 1 > /sys/bus/pci/rescan
```

7. Check device attached
```
./bin/cli device list
```

You can test hotplug by unplug and replug physical device
