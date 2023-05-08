## poseidonos-cli transport

Transport Command for PoseidonOS.

### Synopsis


Transport Command for PoseidonOS. Use this command category when you
create or list a transport. 

Syntax: 
  poseidonos-cli transport [create|list] [flags]
	  

```
poseidonos-cli transport [flags]
```

### Options

```
  -h, --help   help for transport
```

### Options inherited from parent commands

```
      --debug            Print response for debug.
      --fs string        Field separator for the output. (default "|")
      --ip string        Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req         Print request in JSON form.
      --json-res         Print response in JSON form.
      --port string      Set the port number to PoseidonOS for this command. (default "18716")
      --timeout uint32   Timeout for this command in seconds. (default 180)
      --unit             Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS
* [poseidonos-cli transport create](poseidonos-cli_transport_create.md)	 - Create NVMf transport to PoseidonOS.
* [poseidonos-cli transport list](poseidonos-cli_transport_list.md)	 - List NVMf transport to PoseidonOS.

