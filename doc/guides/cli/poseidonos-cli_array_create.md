## poseidonos-cli array create

Create an array for PoseidonOS.

### Synopsis


Create an array for PoseidonOS. 

Syntax: 
	poseidonos-cli array create (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--data-devs | -d) DeviceNameList (--spare | -s) DeviceName [--raid RAID0 | RAID5 | RAID10 | RAID6] 
	[--no-raid]

Example 1 (creating an array with RAID5):  
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
	
Eample 2 (creating an array with RAID6): 	
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID6


```
poseidonos-cli array create [flags]
```

### Options

```
  -a, --array-name string   The name of the array to create.
  -b, --buffer string       The name of device to be used as the buffer.
  -d, --data-devs string    A comma-separated list of devices to be used as the data devices.
                            When the capacities of the data devices are different, the total capacity
                            of this array will be truncated based on the smallest one.
  -h, --help                help for create
  -n, --no-raid             When specified, no RAID will be applied to this array (--raid flag will be ignored).Array with no RAID can have maximum 1 data device(s).
  -r, --raid string         The RAID type of the array to create. RAID5 is used when not specified. (default "RAID5")
  -s, --spare string        The name of device to be used as the spare.
```

### Options inherited from parent commands

```
      --debug            Print response for debug.
      --fs string        Field separator for the output. (default "|")
      --ip string        Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req         Print request in JSON form.
      --json-res         Print response in JSON form.
      --node string      Name of the node to send this command. When both --ip and this flag are specified, this flag is applied only.
      --port string      Set the port number to PoseidonOS for this command. (default "18716")
      --timeout uint32   Timeout for this command in seconds. (Note: array unmount command has 30 minutes timeout.) (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

