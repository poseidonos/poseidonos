## poseidonos-cli volume unmount

Unmount a volume to the host.

### Synopsis


Unmount a volume to the host.

Syntax:
	unmount (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 
	[--subnqn TargetNVMSubsystemNVMeQualifiedName]

Example: 
	poseidonos-cli volume unmount --volume-name Volume0 --array-name Volume0
	
         

```
poseidonos-cli volume unmount [flags]
```

### Options

```
  -a, --array-name string    The name of the array where the volume belongs to.
      --force                Force to unmount this volume.
  -h, --help                 help for unmount
  -v, --volume-name string   The name of the volume to unmount.
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

