## poseidonos-cli volume set-property

Set the properties of a volume.

### Synopsis


Set the properties of a volume.

Syntax: 
	poseidonos-cli volume set-property (--volume-name | -v) VolumeName 
	(--array-name | -a) ArrayName (--new-volume-name | -n) VolumeName
	[--primary-volume] [--secondary-volume]

Example: 
	poseidonos-cli volume set-property --volume-name Volume0 --array-name volume0 
	--new-volume-name NewVolume0 --primary-volume


```
poseidonos-cli volume set-property [flags]
```

### Options

```
  -a, --array-name string        The name of the array where the volume belongs to.
  -h, --help                     help for set-property
  -n, --new-volume-name string   The new name of the volume.
      --primary-volume           If specified, the volume will be set to a primary volume. This flag cannot be set with --secondary-volume
      --secondary-volume         If specified, the volume will be set to a secondary volume for HA.
  -v, --volume-name string       The name of the volume to set the property.
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

