## poseidonos-cli array create

Create an array for PoseidonOS.

### Synopsis


Create an array for PoseidonOS. 

Syntax: 
	poseidonos-cli array create (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--data-devs | -d) DeviceNameList (--spare | -s) DeviceName [--raid RaidType] .

Example: 
	poseidonos-cli array create --array-name Array0 --buffer device0 
	--data-devs nvme-device0,nvme-device1,nvme-device2,nvme-device3 --spare nvme-device4 --raid RAID5
          

```
poseidonos-cli array create [flags]
```

### Options

```
  -a, --array-name string   The name of the array to create.
  -b, --buffer string       The name of device to be used as the buffer.
  -d, --data-devs string    A comma-separated list of devices to be used as the data devices.
  -h, --help                help for create
  -r, --raid string         The RAID type of the array to create. RAID5 is used when not specified. (default "RAID5")
  -s, --spare string        The name of device to be used as the spare.
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

