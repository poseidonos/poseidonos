## poseidonos-cli wbt

White box test (WBT) commands for Poseidon OS

### Synopsis


Send WBT name and arguments to Poseidon OS and get a result fommated by JSON.

You can set IPv4 address and the port number to Poseidon OS confiruing config.yaml file or flags.
Default values are as below:
	IP   : 127.0.0.1
	Port : 18716


	  

```
poseidonos-cli wbt [testname] [flags]
```

### Options

```
      --access string      set access
      --array string       set array
      --cdw10 string       set cdw10
      --cdw11 string       set cdw11
      --cdw12 string       set cdw12
      --cdw13 string       set cdw13
      --cdw14 string       set cdw14
      --cdw15 string       set cdw15
      --cns string         set cns
  -n, --count string       set count
  -d, --dev string         set dev
      --fd string          set fd
  -h, --help               help for wbt
  -i, --input string       set input
      --integrity string   set integrity
      --lba string         set lba
      --lbaf string        set lbaf
      --loc string         set loc
      --lsid string        set lsid
      --ms string          set ms
      --name string        set name
      --normal string      set normal
      --nsid string        set nsid
      --offset string      set offset
      --op string          set op
      --operation string   set operation
  -o, --output string      set output
      --pattern string     set pattern
      --pi string          set pi
      --pil string         set pil
      --rba string         set rba
      --ses string         set ses
      --size string        set size
      --urgent string      set urgent
      --volume string      set volume
      --vsid string        set vsid
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli](poseidonos-cli.md)	 - poseidonos-cli - A command-line interface for PoseidonOS

