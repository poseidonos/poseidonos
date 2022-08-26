## poseidonos-cli devel resetmbr

Reset MBR information of PoseidonOS.

### Synopsis


Reset MBR information of PoseidonOS (Previously: Array Reset command).
Use this command when you need to remove the all the arrays and 
reset the states of the devices. 

Syntax:
	poseidonos-cli devel resetmbr
          

```
poseidonos-cli devel resetmbr [flags]
```

### Options

```
  -h, --help   help for resetmbr
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

* [poseidonos-cli devel](poseidonos-cli_devel.md)	 - Commands for PoseidonOS developers.

