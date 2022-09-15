## poseidonos-cli array

Array command for PoseidonOS.

### Synopsis

Array command for PoseidonOS. Use this command to create, delete, and control arrays.

Syntax: 
  poseidonos-cli array [create|delete|mount|unmount|list|addspare|rmspare|autocreate] [flags]

Example (to create an array):
  poseidonos-cli array create --array-name array0 --buffer uram0 --data-devs nvme0,nvme1,nvme2,nvme3 --spare nvme4
	  

```
poseidonos-cli array [flags]
```

### Options

```
  -h, --help   help for array
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

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS
* [poseidonos-cli array addspare](poseidonos-cli_array_addspare.md)	 - Add a device as a spare to an array.
* [poseidonos-cli array autocreate](poseidonos-cli_array_autocreate.md)	 - Automatically create an array for PoseidonOS.
* [poseidonos-cli array create](poseidonos-cli_array_create.md)	 - Create an array for PoseidonOS.
* [poseidonos-cli array delete](poseidonos-cli_array_delete.md)	 - Delete an array from PoseidonOS.
* [poseidonos-cli array list](poseidonos-cli_array_list.md)	 - List arrays of PoseidonOS or display information of an array.
* [poseidonos-cli array mount](poseidonos-cli_array_mount.md)	 - Mount an array to PoseidonOS.
* [poseidonos-cli array rebuild](poseidonos-cli_array_rebuild.md)	 - Instantly trigger the rebuild for an array.
* [poseidonos-cli array replace](poseidonos-cli_array_replace.md)	 - Replace a data device with an available spare device in array.
* [poseidonos-cli array rmspare](poseidonos-cli_array_rmspare.md)	 - Remove a spare device from an array of PoseidonOS.
* [poseidonos-cli array unmount](poseidonos-cli_array_unmount.md)	 - Unmount an array from PoseidonOS.

