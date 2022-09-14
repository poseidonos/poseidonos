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
      --filetype string    set filetype
  -h, --help               help for wbt
  -i, --input string       set input
      --integrity string   set integrity
      --key string         set key
      --lba string         set lba
      --lbaf string        set lbaf
      --loc string         set loc
      --lsid string        set lsid
      --module string      set module
      --ms string          set ms
      --name string        set name
      --normal string      set normal
      --nsid string        set nsid
      --offset string      set offset
      --op string          set op
  -o, --output string      set output
      --pattern string     set pattern
      --pi string          set pi
      --pil string         set pil
      --rba string         set rba
      --ses string         set ses
      --size string        set size
      --type string        set type
      --urgent string      set urgent
      --value string       set value
      --volume string      set volume
      --vsid string        set vsid
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

