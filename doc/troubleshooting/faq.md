# Frequenty Asked Questions (FAQ)

## Build
#### Q: I got errors when building PoseidonOS on Ubuntu 20.04.

A: Currently, we offically support PoseidonOS on Ubuntu 18.04. On the other versions of Ubuntu, we cannot guarantee the successful build and run of PoseidonOS.

#### Q: Even though I tried the build on Ubuntu 18.04, I got the error messages as below
```
$ sudo ./build_lib.sh
(...)
[ 47%] Performing update step for 'filebench'                                                                                                                                                                                                                                                                                 
CMake Error at /home/martin/poseidonos/lib/filebench-prefix/src/filebench-stamp/filebench-update-.cmake:16 (message):                                          
  Command failed: 1

   '/usr/bin/cmake' '-P' '/home/martin/poseidonos/lib/filebench-prefix/tmp/filebench-gitupdate.cmake'                                                          

  See also
  
    /home/martin/poseidonos/lib/filebench-prefix/src/filebench-stamp/filebench-update-*.log      
```

A: It seems that you have not updated user information of your git.
Try the following:
```
$ git config --global user.name "John Doe"
$ git config --global user.email johndoe@example.com

$ sudo build_lib.sh
$ sudo build_ibofos.sh
```

If this does not work, please see the log using the following command:
```
$ cat /home/martin/poseidonos/lib/filebench-prefix/src/filebench-stamp/filebench-update-*.log
```

#### Q: pkgdep.sh tries to install Go-lang even if go14 or a new version is available.
A: Currently, PoseidonOS is assuming that Go1.14.4 is installed in the default path (/usr/local/go).

## System
#### Q: I have rebooted the server. Should I execute PoseidonOS again?

A: Currently, PoseidonOS does not support auto start-up after reboot (i.e., no initd or systemd configuration). You should execute PoseidonOS again manually after reboot. Please follow the steps described in Learning POS Commands.

#### Q: PoseidonOS has been terminated unexpectedly for a reason. How can I run it again with the same array configuration?**

A: When PoseidonOS has not been terminated ungracefully, you should execute it again. And then, you should configure the arrays again as follows

1. Create urams: currently, you need to create urams again using the following command:
    ```
    $ poseidonos-cli device create --device-name your-uram-name --device-type uram --block-size your-block-size --num-blocks your-num-blocks
    ```
2. Load arrays: the arrays will be automatically loaded when you scan devices using the following command:
    ```
    $ poseidonos-cli device scan
    ```

#### Q: PoseidonOS has been terminated unexpectedly. If ramdisk was used for buffer journal, can I recover the data?

A: If the system has not rebooted since the last crash of PoseidonOS, PoseidonOS is still able to start up by backing up ramdisk and restoring it. Run script ($poseidonos-path/script/backup_latest_hugeages_for_uram.sh) to back up uram data of huge pages before the step 1, 2, 3, and, 4, to load existing array from MBR as described in Learning POS Commands.

Please refer to "How to recover data from ungraceful shutdown?" in Learning Best Practices for further details.

#### Q: Where is the log file of PoseidonOS?

A: PoseidonOS logs events in three sinks: console, major, and minor sinks. In console sink, all the events are printed out as stdout. In major and minor sinks, the events of which levels are higher than debug and warn are logged to /var/log/pos/pos_major.log and /var/log/pos/pos.log, respectively.

#### Q: How large can be a PoseidonOS log file?

A: Similar to other logging system, PoseidonOS uses a rotating file logger. The size and number of rotating files depend on the configuration. You can set them by changing logger.logfile_size_in_mb and logger.logfile_rotation_count in /etc/pos/pos.conf.

 Note: PoseidonOS does not automatically back up the log files; we recommend you to create extra copies of the log files. 



#### Q: Can we parse an ordered-list of PoseidonOS events?

A: PoseidonOS provides logging events in JSON form for structured logging.

#### Q: Can two initiators share single PoseidonOS volume? 

A: Yes, it can. the NVMe-oF specification defines that functionality as namespace sharing. Please refere to the specification document for further detail. 

When your application utilizes namespace sharing, it should coordinate I/Os to the same volume; NVMe or NVMe-oF interface does not guarantee the order of write requests. Therefore, there may exist a consistency problem because of concurrent write operations. 

#### Q: Can a single initiator have multiple I/O paths to a single PoseidonOS volume?

A: Yes, it can. Such feature is called 'multipath I/O'. If your application is required to tolerate any failure in an NVMe-oF I/O path, multipath I/O can be a good solution. 

#### Q: When should I execute SCANDEVICE command? Does not PoseidonOS execute the command periodically?

A: Whenever you create/delete a buffer device or attach/detach an NVMe device, you need to execute SCANDEVICE.
```
$ poseidonos-cli device scan
```
The command will refresh the devices in the system, and load arrays if needed. We plan to execute the command automatically.

## Configuration
#### Q: It seems that pos.conf is broken. Can I recover it to the default?

A: If you have the source code of PoseidonOS codes in your local directory, copy $your-poseidonos-path/config/pos.conf to /etc/pos/pos.conf.

It is okay even if you do not have the source code. When you executes PoseidonOS again, it will create '/etc/pos/pos.conf' with the default configuration.

#### Q: I have changed /etc/pos/pos.conf. How can I apply the configuration?

A: You should restart PoseidonOS to apply the new configuration. 

#### Q: How can I change the log level of PoseidonOS?

A: You can change the minimum event level to log using command line interface (CLI) as follows:
```
$ poseidonos-cli logger set-level --level your-log-level
```
Please refer to [SETLEVELCOMMAND](../guides/cli/poseidonos-cli_logger_set-level.md) for more details. 


#### Q: How can I check if the configuration is loaded to PoseidonOS correctly? What if I store the pos.conf in a wrong path or have no permission to it? Is there any way to verify the configuration?

A: Unfortunately, we are not providing such method.

#### Q: Which RAID levels does PoseidonOS support? My application does not require data protection.
A: PoseidonOS supports RAID0, RAID10, and RAID5. If your application requires high performance, you can create PoseidonOS arrays with RAID0. If it requires high reliability, you may create PoseidonOS arrays with RAID5.

## SSD Devices
#### Q: PoseidonOS does not list some of my NVMe SSDs.
You can try the following steps:

1. Please check if the device is mounted correctly on the server.
2. Execute the following script:
    ```
    $ your-poseidonos-path/script/setup_env.sh
    ```
    It will change the kernel drivers of your SSD devices to user-space drivers.
3. Execute SCANDEVICE command as follows
    ```
    $ your-poseidonos-path/bin/poseidonos-cli device scan
    ```

#### Q: Does PoseidonOS support hot detach or hot attach of SSD devices?

A. Yes. PoseidonOS supports both Hot-attach and Hot-detach.

#### Q: I have installed an SSD device into the server SSD slot, but it is not shown on the device list of PoseidonOS.

A. It may be possible that udev_rule was deleted or modified. Try resetting udev_rule to the default value.

## Performance
#### Q: I/O Performance is lower than specified.
Please check the followings:
1. It is possible that the array is in BUSY state, which means that rebuilding is occuring.
2. This may be because of garbage collection. In this case, the performance may increase after the garbage collection is done.

#### Q: I want higher I/O performance during the rebuilding.
A: When you want to reduce the performance interference by rebuilding, you can specify the degree of interference using the SETPROPERTY command. For example:
```
$ poseidonos-cli system set-property --rebuild-impact low
```
Please refer to [SETPROPERTY](../guides/cli/poseidonos-cli_system_set-property.md) for further information.

#### Q: I have changed the rebuilding impact level. When will it be applied?
A: The rebuilding impact is applied right after you execute the command. 

## Command Line Interface (CLI)
#### Q: My command does not work. PoseidonOS replies "POS is busy".
A. It is because PoseidonOS is processing a command (currently, PoseidonOS does not accept multiple requests from CLI). Please execute your command after a while.

#### Q: I would like to parse the output of CLI for some automation. How can I do that?
A: Poseidonos-CLI display the output as grep- and awk-friendly formats. For example, you can grep the information of an array as below:
```
# Displaying the information of "unvme-ns-4" only.
/poseiondonos/bin$ ./poseidonos-cli device list | grep unvme-ns-4
unvme-ns-4     |020                  |0000:ce:00.0   |SYSTEM        |SAMSUNG SSD -Q-                          |1      |3840755982336
```
Please refere to [Advanced POS CLI Usage](../guides/cli/advanced_pos_cli_usage.md) for further details.

## Management Stack
#### Q: I would like to access PoseidonOS via RESTful APIs. What shoud I do?
A: You need to install Management Stack to manage PoseidonOS via RESTful APIs. You can find it here: [Management Stack](https://github.com/poseidonos/poseidonos-gui). Once you install it, you can manage (e.g., create/delete/mount arrays and volumes) PoseidonOS via RESTful APIs and a GUI called poseidon-gui.


## Array
#### Q: Can I change the RAID level of a PoseidonOS array?
A: No, it is not supported. Please create another array with a different RAID level.

#### Q: My PoseidonOS array is in Degraded mode. How can I turn it back to Normal mode?

A: When there is no spare device for rebuild, a PoseidonOS array works in Degraded mode. This means that rebuilding has been paused. Once you add a spare device to the array using the ADDSPARE command, PoseidonOS will continue to rebuild the array. The array will become Normal mode when the rebuilding completes. 

#### Q: I deleted a volume by mistake. Can I recover the volume or data?
A: Unfortunately, no. POS does not provide any method to recover a deleted volume or data.