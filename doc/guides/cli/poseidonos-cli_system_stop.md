## poseidonos-cli system stop

Stop PoseidonOS.

### Synopsis


Stop PoseidonOS.

Syntax:
	poseidonos-cli system stop
          

```
poseidonos-cli system stop [flags]
```

### Options

```
      --force   Force to stop PoseidonOS.
  -h, --help    help for stop
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

* [poseidonos-cli system](poseidonos-cli_system.md)	 - System commands for PoseidonOS.

