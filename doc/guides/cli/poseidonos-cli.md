## poseidonos-cli

poseidonos-cli - A command-line interface for PoseidonOS

### Synopsis

poseidonos-cli - A command-line interface for PoseidonOS

	PoseidonOS command-line interface (PoseidonOS CLI) is a management tool for PoseidonOS.
	Using PoseidonOS CLI, you can start/stop PoseidonOS and manage arrays, devices, and volumes of PoseidonOS.
	
	Type --help or -h with the command to see detailed information about each command.

Syntax: 
  poseidonos-cli [global-flags] commands subcommand [flags]
		

```
poseidonos-cli [flags]
```

### Options

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
  -h, --help          help for poseidonos-cli
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
      --version       Display the version information.
```

### SEE ALSO

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.
* [poseidonos-cli completion](poseidonos-cli_completion.md)	 - Generate completion script
* [poseidonos-cli devel](poseidonos-cli_devel.md)	 - Commands for PoseidonOS developers.
* [poseidonos-cli device](poseidonos-cli_device.md)	 - Device commands for PoseidonOS.
* [poseidonos-cli logger](poseidonos-cli_logger.md)	 - Logger commands for PoseidonOS.
* [poseidonos-cli qos](poseidonos-cli_qos.md)	 - QoS commands for PoseidonOS.
* [poseidonos-cli subsystem](poseidonos-cli_subsystem.md)	 - Subsystem Command for PoseidonOS.
* [poseidonos-cli system](poseidonos-cli_system.md)	 - System commands for PoseidonOS.
* [poseidonos-cli telemetry](poseidonos-cli_telemetry.md)	 - Telemetry commands for PoseidonOS.
* [poseidonos-cli volume](poseidonos-cli_volume.md)	 - Volume commands for PoseidonOS.
* [poseidonos-cli wbt](poseidonos-cli_wbt.md)	 - White box test (WBT) commands for Poseidon OS

