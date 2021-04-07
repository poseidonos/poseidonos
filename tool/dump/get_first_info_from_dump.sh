#!/bin/bash
#decompress dump file

PID=`ps -ef | egrep "*/ibofos$" | awk '{print $2}'`
cd $(dirname $(realpath $0))
cd ../../
CURR_PATH=`pwd`
IBOFOS_CORE="$CURR_PATH""/bin/ibofos.core"
cd -
CORE_DIR="/etc/ibofos/core/"

rm -rf attach_or_load.gdb
if [ -z $PID ];then
    echo "There are no alive ibofos process found in the system"
    PID="$CORE_DIR"${IBOFOS_CORE//"/"/"!"}
    if [ ! -f $PID ];then
        PID="$CORE_DIR""ibofos.core"
    fi
    echo $PID
    echo "core ""$PID" > attach_or_load.gdb
else
    echo "attach ""$PID" > attach_or_load.gdb
fi

rm call_stack.info pending_io.info -rf 
cat get_first_info_from_dump.gdb >> attach_or_load.gdb
gdb $CURR_PATH/bin/ibofos -x attach_or_load.gdb -batch

cd -;



