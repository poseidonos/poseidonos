## poseidonos-cli logger set-preference

Set the preferences (e.g., format) of logger.

### Synopsis


Set the preferences (e.g., format) of logger.

Syntax:
	poseidonos-cli logger set-preference [--json BooleanValue]
          

```
poseidonos-cli logger set-preference [flags]
```

### Options

```
  -h, --help                        help for set-preference
  -s, --structured-logging string   When specified as true, PoseidonOS will log the events in JSON form for structured logging.
                                    Otherwise, the events will be logged in plain text form. (default "false")
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

* [poseidonos-cli logger](poseidonos-cli_logger.md)	 - Logger commands for PoseidonOS.

