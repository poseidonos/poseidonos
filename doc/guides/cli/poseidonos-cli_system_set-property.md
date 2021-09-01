## poseidonos-cli system set-property

Set the property of PoseidonOS.

### Synopsis

Set the property of PoseidonOS.

Syntax:
	poseidonos-cli system set-property [--rebuild-impact "highest"|"higher"|"high"|"medium"|"low"|"lower"|"lowest"] .

Example (To set the impact of rebuilding process on the I/O performance to low):
	poseidonos-cli system set-property --rebuild-impact low.
          

```
poseidonos-cli system set-property [flags]
```

### Options

```
  -h, --help                    help for set-property
      --rebuild-impact string   The impact of rebuilding process on the I/O performance
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

* [poseidonos-cli system](poseidonos-cli_system.md)	 - System commands for PoseidonOS.

