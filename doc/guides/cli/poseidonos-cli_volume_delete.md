## poseidonos-cli volume delete

Delete a volume from PoseidonOS.

### Synopsis


Delete a volume from an array in PoseidonOS.

Syntax:
	poseidonos-cli volume delete (--volume-name | -v) VolumeName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli volume delete --volume-name Volume0 --array=name Array0
	
          

```
poseidonos-cli volume delete [flags]
```

### Options

```
  -a, --array-name string    The Name of the array where the volume belongs to.
      --force                Force to delete the volume (volume must be unmounted first).
  -h, --help                 help for delete
  -v, --volume-name string   The Name of the volume to delete.
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

