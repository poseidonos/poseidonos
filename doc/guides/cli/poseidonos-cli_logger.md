## poseidonos-cli logger

Logger commands for PoseidonOS.

### Synopsis


Logger commands for PoseidonOS. Use this command category to
control and display information about logger. 

Syntax: 
  poseidonos-cli logger [set-level|get-level|apply-filter|info|set-preference] [flags]

Example (to get the current log level):
  poseidonos-cli logger get-level
	  

```
poseidonos-cli logger [flags]
```

### Options

```
  -h, --help   help for logger
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
* [poseidonos-cli logger apply-filter](poseidonos-cli_logger_apply-filter.md)	 - Apply a filtering policy to logger.
* [poseidonos-cli logger get-level](poseidonos-cli_logger_get-level.md)	 - Get the filtering level of logger.
* [poseidonos-cli logger info](poseidonos-cli_logger_info.md)	 - Display the current preference of logger.
* [poseidonos-cli logger set-level](poseidonos-cli_logger_set-level.md)	 - Set the filtering level of logger.
* [poseidonos-cli logger set-preference](poseidonos-cli_logger_set-preference.md)	 - Set the preferences (e.g., format) of logger.

