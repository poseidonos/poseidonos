# Trigger Core Dump


./trigger_core_dump.sh collects the dump and debug information from running poseidonos or crashed one. This compresses core files and useful log files for debug.
./load_core.sh load the dump by gdb command line. The compressed files obtained from __another host__ can be analyzed in __this host__ with load_core.sh

##precondition
gdb or another debug tool should not be executed.

##How to trigger getting dump?

1) Before starting test(script/start_poseidonos.sh), please run the script 
```
      cd script
      ./setup_env.sh
```


2) you can get the crash dump with this command with running poseidonos or already killed poseidonos process.
  - Option 1 : Running Process with killing process (Recommended)
    ```    
      # ./trigger_core_dump.sh triggercrash
    ```
    "triggercrash" option kill the running process and get the crash dump from host. 
    crashed dump size is small and makes process kill, but can get dump fast. 
<br/>

  - Option 2 : Crashed Process (Recommended)
    you also get latest crash core dump with already killed process from segmentation fault.
    ```
      - ./trigger_core_dump.sh crashed
    ```
<br/>

  - Option 3 : Running Process without killing process
    you also get normal core dump with "gcore". (64G~128G)
    ```
      - ./trigger_core_dump.sh gcore
    ```  
    This option will not kill the process, but it takes much longer time than another option.
<br/>

  - Option 4 : Collect only log
    if you want to get only the poseidonos log (dmesg / poseidonos log / syslog) please use "logonly"
    This logs are included all options' compressed file.

      * stdout log from poseidonos will get if you execute poseidonos with "./start_poseidonos.sh"
      - ./trigger_core_dump.sh logonly
<br/>

##How to load dump?

1) Output file (tar.gz file including core dump file) from trigger_core_dump.sh can be loaded by load_dump.sh even in other host. 
   First, you need to change or add this script to ~/.gdbinit as below to analyze the c++ stl materials. you need check which gcc version and put proper version. (you can check : ls /usr/share/gcc-9 folder exists or not)
    ```
     python
     import sys
     sys.path.insert(0, '/usr/share/gcc-9/python/');
     from libstdcxx.v6.printers import register_libstdcxx_printers
     register_libstdcxx_printers (None)    
     ```
<br/>

2) Execute load_dump.sh
   [compressed] excludes the "tar.gzxx". please input prefix of compressed files.
   ```
          ./load_dump.sh [compressed]
   ```
   if compressed file name is "poseidonos.core.20220223_072617.crashed.tar.gzxx"
   ```
          ./load_dump.sh poseidonos.core.20220223_072617.crashed
   ```

