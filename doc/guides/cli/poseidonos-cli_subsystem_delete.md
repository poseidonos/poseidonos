## poseidonos-cli subsystem delete

Delete a subsystem from PoseidonOS.

### Synopsis


Delete a subsystem from PoseidonOS.

Syntax:
	poseidonos-cli subsystem delete (--subnqn | -q) SubsystemNQN

Example:
	poseidonos-cli subsystem delete --subnqn nqn.2019-04.pos:subsystem
    

```
poseidonos-cli subsystem delete [flags]
```

### Options

```
      --force           Force to delete this subsystem.
  -h, --help            help for delete
  -q, --subnqn string   NQN of the subsystem to delete.
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

* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.

