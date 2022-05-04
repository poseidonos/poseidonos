set print elements 0
set print pretty
set height unlimited
source ./posgdb.py
posgdb log memory

set logging file call_stack.info
set logging on
thread apply all bt
set logging off

