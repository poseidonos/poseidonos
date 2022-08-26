## poseidonos-cli subsystem

Subsystem Command for PoseidonOS.

### Synopsis


Subsystem Command for PoseidonOS. Use this command category when you
create or delete a subsystem. 

Syntax: 
  poseidonos-cli subsystem [add-listener|create|delete|list|create-transport] [flags]
	  

```
poseidonos-cli subsystem [flags]
```

### Options

```
  -h, --help   help for subsystem
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
* [poseidonos-cli subsystem add-listener](poseidonos-cli_subsystem_add-listener.md)	 - Add a listener to an NVMe-oF subsystem
* [poseidonos-cli subsystem create](poseidonos-cli_subsystem_create.md)	 - Create an NVMe-oF subsystem to PoseidonOS.
* [poseidonos-cli subsystem create-transport](poseidonos-cli_subsystem_create-transport.md)	 - Create NVMf transport to PoseidonOS.
* [poseidonos-cli subsystem delete](poseidonos-cli_subsystem_delete.md)	 - Delete a subsystem from PoseidonOS.
* [poseidonos-cli subsystem list](poseidonos-cli_subsystem_list.md)	 - List subsystems from PoseidonOS.

