set print elements 0
set print pretty
set height unlimited
source ./ibof_gdb.py
ibofgdb log memory

set logging file call_stack.info
set logging on
thread apply all bt
set logging off

set logging file pending_io.info
set logging on
ibofgdb pending io
ibofgdb pending ubio
set logging off

