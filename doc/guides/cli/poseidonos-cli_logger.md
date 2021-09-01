## poseidonos-cli logger

Logger commands for PoseidonOS.

### Synopsis

Logger commands for PoseidonOS.

Syntax: 
  poseidonos-cli logger [set-level|get-level|apply-filter|info]

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
      --debug         Print response for debug
      --fs string     Field separator for the output (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command (default "127.0.0.1")
      --json-req      Print request in JSON form
      --json-res      Print response in JSON form
      --port string   Set the port number to PoseidonOS for this command (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS [version 0.7]
* [poseidonos-cli logger apply-filter](poseidonos-cli_logger_apply-filter.md)	 - Apply a filtering policy to the logger.
* [poseidonos-cli logger get-level](poseidonos-cli_logger_get-level.md)	 - Get the filtering level of logger.
* [poseidonos-cli logger info](poseidonos-cli_logger_info.md)	 - Get the current settings of the logger.
* [poseidonos-cli logger set-level](poseidonos-cli_logger_set-level.md)	 - Set the filtering level of logger.

