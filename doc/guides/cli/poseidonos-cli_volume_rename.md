## poseidonos-cli volume rename

Rename a volume of PoseidonOS.

### Synopsis


Rename a volume of PoseidonOS.

Syntax:
	poseidonos-cli volume rename (--volume-name | -v) VolumeName (--array-name | -a) ArrayName 
	(--new-volume-name | -n) VolumeName

Example (renaming a volume): 
	poseidonos-cli volume rename --volume-name OldVolumeName --array-name Array0 --new-volume-name NewVolumeName
          

```
poseidonos-cli volume rename [flags]
```

### Options

```
  -a, --array-name string        The Name of the array of the volume to change.
  -h, --help                     help for rename
  -n, --new-volume-name string   The new name of the volume.
  -v, --volume-name string       The Name of the volume to change its name.
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
      --timeout uint32   Timeout for this command in seconds. (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.

