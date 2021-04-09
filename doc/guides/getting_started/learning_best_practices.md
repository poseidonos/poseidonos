How to Stop POS Gracefully?
========
```
# Stop the user application. This may differ by application
initiator$ sudo service stop client-service-name
 
# Unmount the user's file system (assumption: /dev/nvme-0n1 is mounted on /mnt)
initiator$ sudo umount /mnt
 
# Unmount the POS volume (assumption: the volume name is vol1 and created within Array called POSArray)
target$ cli volume unmount --name vol1 --array POSArray
 
# Unmount the POS array (assumption: POS does not support multi-arrays yet)
target$ cli system unmount
 
# Kill the POS
target$ cli system exit
```

If you don't stop user I/Os before unmounting POS volume or array, there is a risk of write failures. 


How to Recover Data from Ungraceful Shutdown?
========
Even if POS has crashed ungracefully, there is a chance of recovering data as long as the following conditions were met at the time of the crash:
* POS was running with journaling enabled.
* POS was using a ram disk as a buffer device.
* The underlying OS has been up and running even after the crash.

The solution is to copy the whole bytes from the hugepage memory area to shared memory region (i.e., tmpfs) and perform recovery operation by loading the array. The followings are the steps:

```
# POS has crashed, but OS remains healthy
 
# Dump hugepage contents to shared memory region. Assume you're at the root of POS repository
$ sudo ./script/backup_latest_hugepages_for_uram.sh
 
# Bring up POS (refer to Getting Started for further details)
$ sudo ./script/start_ibofos.sh
 
# Create write buffer. Please make sure the same parameters should be used as before the crash.
$ ./lib/spdk-19.10/scripts/rpc.py bdev_malloc_create -b uram0 8192 512
 
# Scan the devices
$ bin/cli device scan
 
# Load the array. Please make sure the same array name should be used as in the previous execution.
$ bin/cli array load --name {ArrayName}
 
# POS will perform the recovery and become mountable.
```
