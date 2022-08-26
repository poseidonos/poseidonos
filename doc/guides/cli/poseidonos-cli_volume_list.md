## poseidonos-cli volume list

List volumes of an array or display information of a volume.

### Synopsis


List volumes of an array or display information of a volume. Please note that the 'Remaining' field in the output 
shows an internal state that represents how many block addresses of the given volume are yet to be mapped. It
should not be interpreted as the file system free space.

Syntax:
	poseidonos-cli volume list (--array-name | -a) ArrayName [(--volume-name | -v) VolumeName]

Example1 (listing volumes of an array):
	poseidonos-cli volume list --array-name Array0

Example2 (displaying a detailed information of a volume):
	poseidonos-cli volume list --array-name Array0 --volume-name Volume0
          

```
poseidonos-cli volume list [flags]
```

### Options

```
  -a, --array-name string    The name of the array of volumes to list
  -h, --help                 help for list
  -v, --volume-name string   The name of the volume of the array to list.
                             When this is specified, the detailed information
                             		of this volume will be displayed.
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

