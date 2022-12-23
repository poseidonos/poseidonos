#!/bin/bash
#decompress dump file

PID=`ps -ef | egrep "*/poseidonos$" | awk '{print $2}'`
cd $(dirname $(realpath $0))
cd ../../
CURR_PATH=`pwd`
IBOFOS_CORE="$CURR_PATH""/bin/poseidonos.core"
cd -
CORE_DIR="/etc/pos/core/"

rm -rf attach_or_load.gdb
if [ -z $PID ];then
    echo "There are no alive poseidonos process found in the system"
    PID="$CORE_DIR"${IBOFOS_CORE//"/"/"!"}
    if [ ! -f $PID ];then
        PID="$CORE_DIR""poseidonos.core"
    fi
    echo $PID
    echo "core ""$PID" > attach_or_load.gdb
else
    echo "attach ""$PID" > attach_or_load.gdb
fi

rm call_stack.info pending_io.info -rf 
cat get_first_info_from_dump.gdb >> attach_or_load.gdb
gdb $CURR_PATH/bin/poseidonos -x attach_or_load.gdb -batch 1>/dev/null
echo ""
echo "Complete"
cd -;



