## poseidonos-cli array autocreate

Automatically create an array for PoseidonOS.

### Synopsis


Automatically create an array for PoseidonOS with the number of 
devices the user specifies. Use this command when you do not care 
which devices are included to the array. This command will automatically
create an array with the devices in the same NUMA.

Syntax: 
	poseidonos-cli array autocreate (--array-name | -a) ArrayName (--buffer | -b) DeviceName 
	(--num-data-devs | -d) Number [(--num-spare | -s) Number] [--raid RaidType]
	[--no-raid] [--no-buffer]

Example: 
	poseidonos-cli array autocreate --array-name Array0 --buffer uram0 --num-data-devs 3 --num-spare 1
          

```
poseidonos-cli array autocreate [flags]
```

### Options

```
  -a, --array-name string   The name of the array to create.
  -b, --buffer string       The name of device to be used as buffer.
  -h, --help                help for autocreate
  -n, --no-raid             When specified, no RAID will be applied to this array (--raid flag will be ignored).Array with no RAID can have maximum 1 data device(s).
  -d, --num-data-devs int   The number of of the data devices. POS will select the data
                            devices in the same NUMA as possible.
  -s, --num-spare int       Number of devices to be used as the spare.
  -r, --raid string         The RAID type of the array to create. RAID5 is used when not specified. (default "RAID5")
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

