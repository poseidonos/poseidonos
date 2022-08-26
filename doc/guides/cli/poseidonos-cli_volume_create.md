## poseidonos-cli volume create

Create a volume from an array in PoseidonOS.

### Synopsis


Create a volume from an array in PoseidonOS.

Syntax: 
	poseidonos-cli volume create (--volume-name | -v) VolumeName 
	(--array-name | -a) ArrayName --size VolumeSize [--maxiops" IOPS] [--maxbw Bandwidth] [--iswalvol]

Example: 
	poseidonos-cli volume create --volume-name Volume0 --array-name volume0 
	--size 1024GB --maxiops 1000 --maxbw 100GB/s --iswalvol


```
poseidonos-cli volume create [flags]
```

### Options

```
  -a, --array-name string    The name of the array where the volume is created from.
  -h, --help                 help for create
      --iswalvol             If specified, the volume to be created will be a wal volume for HA.
      --maxbw uint           The maximum bandwidth for the volume in MB/s.
      --maxiops uint         The maximum IOPS for the volume in Kilo.
      --size string          The size of the volume in B, K, KB, G, GB, ... (binary units (base-2))
                             If you do not specify the unit, it will be B in default. (Note: the size must be an integer number.) (default "0")
      --uuid string          UUID for the volume to be created.
  -v, --volume-name string   The name of the volume to create.
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

* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.

