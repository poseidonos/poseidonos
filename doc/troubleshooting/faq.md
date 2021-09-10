# Frequenty Asked Questions (FAQ)
## System
#### Q: I have rebooted the server. What should I do to run POS?

A: POS isn't configured for auto start-up (i.e., no initd or systemd configuration yet). It requires manual environment setup and binary execution. Simply, you may want to follow the step 1, 2, 3, 4, and 5b to load existing array as described in Learning POS Commands. 



#### Q: POS has been killed for some reason. How can I re-run POS?**

A: If POS didn't exit gracefully, POS array cannot be loaded so you need to create an array again. If POS is shut down gracefully, you may want to follow the step 1, 2, 3, 4, and 5b to load existing array as described in Learning POS Commands.



#### Q: POS has been killed in some reason. If ramdisk was used for buffer journal, can I recover the data?

A: If the system has not rebooted since the POS crash, POS is still able to start up by backing up ramdisk and restoring it. Run the script for backing up uram data of huge pages ./script/backup_latest_hugeages_for_uram.sh prior to the step 1, 2, 3, and, 4, to load existing array from MBR as described in Learning POS Commands. Please refer to "How to recover data from ungraceful shutdown?" in Learning Best Practices for further details.



#### Q: Where can I find the log file of POS?

A: It is located at /var/log/pos/pos.log



#### Q: What is the retention policy for POS log?

A: POS rotates logs based on the size configuration logger.logfile_size_in_mb and retains N-most recent files only based on the count configuration logger.logfile_rotation_count. POS does not offer a solution to back up the log files. It is up to users to create extra copies of them to meet their retention policy. 



#### Q: Is there a way to extract ordered list of events observed by POS?

A: Parsing the log file is the only way at the moment. 



#### Q: Can two initiators share the same POS volume? 

A: Yes, they can. According to the section 1.4.1 in NVMe spec and the section 1.5.2 in NVMe-oF spec, such sharing is called "namespace sharing". It says "Namespace sharing refers to the ability for two or more hosts to access a common shared namespace using different NVM Express controllers" and gives heads up on concurrent accesses to the same volume: "Concurrent access to a shared namespace by two or more hosts requires some form of coordination between hosts.". Simply, your application should coordinate I/Os to the same volume since NVMe or NVMe-oF does not guarantee any write ordering and may lead to consistency issues upon concurrent writes. 



#### Q: Can one initiator have multiple I/O paths to the same POS volume?

A: Yes, it can. Such feature is called "multipath I/O". If your application has to tolerate any failure in NVMe-oF I/O path, adopting multipath configuration could be one option. 



#### Q: When should I run SCANDEVICE command? Does POS run periodically by any chance?

A: Whenever you create/delete a buffer device or attach/detach NVMe devices, you would want to run SCANDEVICE so that POS refreshes its internal devices information. The command should be run manually at the moment.



## Configuration
#### Q: I have messed up with my configuration file. How can I revert it back to the default?

A: If you have the POS codes locally, copy $POS_HOME/config/pos.conf to /etc/pos/pos.conf



#### Q: I have blown away my configuration file. I don't even have the POS codes checked out locally.

A: When POS starts up, it will create a default configuration file if "/etc/pos/pos.conf" does not exist.



#### Q: I have changed my configuration at /etc/pos/pos.conf. How should I reload the configuration?

A: POS needs to be restarted. 



#### Q: How do I change the log level of POS?

A: Use CLI to adjust to the level you want, e.g., "cli logger set_level --level debug". Please refer to Logger Commands for more details. 



#### Q: How do I check whether the configuration file is properly loaded into POS? How do I know whether I put the conf file at the right directory with right permission? Do we have a command to verify the config?

A: POS does not offer such tool/API at the moment.



#### Q: Can I turn off RAID entirely? My application deals with non system-of-records (e.g., intermediate analytics data, map/reduce, external sorting, table merge, ...) and doesn't care much about the data protection.

A: POS uses either RAID1 or RAID5 for a Partition, but does not provide an option to turn it off. 



## SSD
#### Q: I can't find any or specific NVMe SSDs.

A1) Please check if the device is well mounted on the server

A2) Change the kernel driver to user-space driver by executing $POS_REPO/script/setup_env.sh 

A3) Perform SCANDEVICE command through CLI



#### Q: Does it support Hot-detach and Hot-attach of SSD?

A. Both Hot-attach and Hot-detach are supported.



#### Q: I have inserted SSD into the slot, but it is not visible on the device list

A. udev_rule may have disappeared or been modified. Reset udev_rule to the default provided value



## Performance
#### Q: Performance is lower than specified.

A1) Please check if it is in BUSY state. It may be in rebuild or degraded state.

A2) This may be a situation where garbage collection occurs frequently.



#### Q: I want to improve the user I/O performance during the rebuilding.

A: Performance can be adjusted between rebuild and user I/O through perf_impact command.



#### Q: If I change the perf level of rebuilding, does it get reflected right away or in the next rebuilding?

A: It is reflected right away. 



## CLI
#### Q: POS is busy is returned upon CLI command

A. The current previous command has not been processed. Please try again in a moment.



## Array
#### Q: Can I change the RAID method of the data partition?

A: It is not currently supported.



#### Q: POS array is in a degraded mode. What can I do to make it back to Normal?

A: The reason why POS remains in a degraded mode for long could be because there is no spare device to use for rebuild. If you add spare device to the array, rebuild could resume and POS can get back to normal once the rebuild is completed. 



#### Q: I have accidentally deleted a volume. Is there a way to recover by any chance?

A: No, there isn't. Currently, POS doesn't provide a way to restore a deleted volume. 