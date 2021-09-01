## poseidonos-cli subsystem list

Get a list of subsystems from PoseidonOS.

### Synopsis

Get a list of subsystems from PoseidonOS.

Syntax:
	poseidonos-cli subsystem list [(--subnqn | -q) SubsystemNQN]

Example 1 (listing all subsystems):
	poseidonos-cli subsystem list

Example 2 (listing a specific array):
	poseidonos-cli subsystem list --subnqn nqn.2019-04.pos:subsystem
    

```
poseidonos-cli subsystem list [flags]
```

### Options

```
  -h, --help            help for list
  -q, --subnqn string   NQN of the subsystem to list spec
```

### Options inherited from parent commands

```
      --debug         Print response for debug
      --fs string     Field separator for the output (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command (default "127.0.0.1")
      --json-req      Print request in JSON form
      --json-res      Print response in JSON form
      --port string   Set the port number to PoseidonOS for this command (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity
```

### SEE ALSO

* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.

