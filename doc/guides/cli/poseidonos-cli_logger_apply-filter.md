## poseidonos-cli logger apply-filter

Apply a filtering policy to logger.

### Synopsis


Apply a filtering policy to logger.

  - Filtering file: when executing this command, PoseidonOS reads a filtering policy 
  stored in a file. You can set the file path of the filter in the PoseidonOS configuration 
  (the default path is /etc/conf/filter). If the file does not exist, you can create one.
  
  - Filter file format (EBNF):
  [ include: FilterList ]
  [ exclude: FIlterList ]
  FilterList = FilterNumber | FilterNumber,FilterList
  FilterNumber = { ( Number ) | ( Range ) }
  Number = { Digit }
  Range = Number-Number
  
  - Filter file example:
  include: 1002,1005,6230,2000-3000
  exclude: 1006,5003,8000-9000

Syntax:
  poseidonos-cli logger apply-filter
          

```
poseidonos-cli logger apply-filter [flags]
```

### Options

```
  -h, --help   help for apply-filter
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

