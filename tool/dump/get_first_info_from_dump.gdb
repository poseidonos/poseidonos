set print elements 0
set print pretty
set height unlimited
source ./posgdb.py
posgdb log memory

set logging file debug.info
set logging overwrite on
set logging on
posgdb debug info
set logging off

posgdb debug history

set logging file call_stack.info
set logging overwrite on
set logging on
thread apply all bt
set logging off
