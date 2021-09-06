## poseidonos-cli volume create

Create a volume from an array in PoseidonOS.

### Synopsis


Create a volume from an array in PoseidonOS.

Syntax: 
	poseidonos-cli volume create (--volume-name | -v) VolumeName 
	(--array-name | -a) ArrayName --size VolumeSize [--maxiops" IOPS] [--maxbw Bandwidth] .

Example: 
	poseidonos-cli volume create --volume-name Volume0 --array-name volume0 
	--size 1024GB --maxiops 1000 --maxbw 100GB/s
          

```
poseidonos-cli volume create [flags]
```

### Options

```
  -a, --array-name string    The name of the array where the volume is created from.
  -h, --help                 help for create
      --maxbw int            The maximum bandwidth for the volume in MB/s.
      --maxiops int          The maximum IOPS for the volume in Kilo.
      --size string          The size of the volume in B, K, KB, G, GB, ... (binary units (base-2))
                             If you do not specify the unit, it will be B in default. (default "0")
  -v, --volume-name string   The name of the volume to create.
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

* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.

