
# posgdb

posgdb parses debug information from dump and provides useful information

## precondition
   you need to change or add this script to ~/.gdbinit as below to analyze the c++ stl materials. 
   you need check which gcc version and put proper version. 
   (you can check : ls /usr/share/gcc-9 folder exists or not)
```
     python
     import sys
     sys.path.insert(0, '/usr/share/gcc-9/python/');
     from libstdcxx.v6.printers import register_libstdcxx_printers
     register_libstdcxx_printers (None)
```
## Step to use script "posgdb.py"
1) please execute gdb with core files (crashed process) or pid (running process). 
You can also execute gdb with "load_dump.sh" 
   (Please check trigger_core_dump.md)

```
   gdb ./bin/poseidonos [corefile name or pid]
```

2) In gdb command line, please input commands "source posgdb.py"
```
   source posgdb.py
```

3) And, you can get the useful information with "posgdb"
```
   posgdb [command] [subcommand]
```

## posgdb Commands

1) posgdb SingletonInfo
   - show debug info and entry point for global objects.
</br>
2) posgdb pending io
   - show pending ios for every thread and devices
</br>
3) posgdb pending ubio
   - show pending Ubio objects when debug option is on.
</br>
4) posgdb pending iocontext
   - show pending IOContext objects when debug option is on.
</br>
5) posgdb callback [address]
   - show a single callback object with address. Chaining information of callbacks is also given.
</br>
6) posgdb pending callback
   - show pending callbacks when debug option is on. Chaining information of callbacks is also given.
</br>
7) posgdb dumpbuffer [address]
   show object buffer queue obtained by dump module
</br>
8) posgdb log memory
   in memory log. result file will be written on current folder.
</br>
9) posgdb pending object [enum type name]
   show object buffer of "dumpSharedPtr"
   ex ) posgdb pending object IO_CONTEXT
</br>
10) posgdb backend io 
   summary of backend io
</br>
11) posgdb volume io
   summary of volume io
</br>
12) posgdb volume info
   Indicates relation between (spdk bdev, subsystem) <-> (Array, volume Id)
</br>
13) posgdb rba info [array_id] [volume_id] [rba(4k unit)]
   see rba's stripe id and location
</br>
