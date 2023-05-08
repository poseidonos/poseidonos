## poseidonos-cli devel dump-memory-snapshot

Dump a memory snapshot of running PoseidonOS.

### Synopsis


Dump a memory snapshot (core dump) of running PoseidonOS.
Use this command when you need to store the current memory snapshot of
PoseidonOS for debugging purpose. 

Syntax:
	poseidonos-cli devel dump-memory-snapshot --path FilePath
          

```
poseidonos-cli devel dump-memory-snapshot [flags]
```

### Options

```
      --force         Force to dump the memory snapshot.
  -h, --help          help for dump-memory-snapshot
      --path string   The path to store the snapshot
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

* [poseidonos-cli devel](poseidonos-cli_devel.md)	 - Commands for PoseidonOS developers.

