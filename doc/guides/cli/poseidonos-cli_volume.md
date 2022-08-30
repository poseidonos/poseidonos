## poseidonos-cli volume

Volume commands for PoseidonOS.

### Synopsis


	Volume commands for PoseidonOS. Use this command category to control volumes
	or display the information of the volumes. 

Syntax: 
  poseidonos-cli volume [create|delete|mount|unmount|list|rename|mount-with-subsystem] [flags]

Example (to create a volume):
  poseidonos-cli volume create --volume-name Volume0 --array-name Array0 
  --size 1024GB --maxiops 1000 --maxbw 100GB/s
	  

```
poseidonos-cli volume [flags]
```

### Options

```
  -h, --help   help for volume
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
* [poseidonos-cli volume create](poseidonos-cli_volume_create.md)	 - Create a volume from an array in PoseidonOS.
* [poseidonos-cli volume delete](poseidonos-cli_volume_delete.md)	 - Delete a volume from PoseidonOS.
* [poseidonos-cli volume list](poseidonos-cli_volume_list.md)	 - List volumes of an array or display information of a volume.
* [poseidonos-cli volume mount](poseidonos-cli_volume_mount.md)	 - Mount a volume to the host.
* [poseidonos-cli volume mount-with-subsystem](poseidonos-cli_volume_mount-with-subsystem.md)	 - Create a subsystem and add listener automatically. Mount a volume to Host.
* [poseidonos-cli volume rename](poseidonos-cli_volume_rename.md)	 - Rename a volume of PoseidonOS.
* [poseidonos-cli volume set-property](poseidonos-cli_volume_set-property.md)	 - Set the properties of a volume.
* [poseidonos-cli volume unmount](poseidonos-cli_volume_unmount.md)	 - Unmount a volume to the host.

