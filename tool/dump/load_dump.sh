#!/bin/bash
#please enter $1 as pid or [corefile name], $2 as gdb script

#decompress dump file

log_normal(){
    echo -e "\033[32m"$1"\033[0m"
}

log_error(){
    echo -e "\033[31m"$1"\033[0m"
}


ROOT_PATH="$(cd "$(dirname "$0")" && pwd)"
cd $ROOT_PATH

PID=`ps -ef | egrep "*/poseidonos$" | awk '{print $2}'`

if [ $# -eq 0 ];then
    echo "Please indicate dump file name as parameter.."
    exit
fi    

SUFFIX=".tar.gz"
CURR_PATH=".."
LIB_FILE="library.tar.gz"
DUMP_EXTRACT_PATH=$(basename ${1%"$SUFFIX"})
LOG_SUFFIX="log"
mkdir -p $DUMP_EXTRACT_PATH
cd $DUMP_EXTRACT_PATH
IS_LOG_ONLY_COMPRESSED="0"

if [[ $1 == *"log"* ]]; then
    IS_LOG_ONLY_COMPRESSED="1"
fi

if [ -f "$DUMP_EXTRACT_PATH" ];then
    log_normal "##### Already Extracted core file exists ######"
else    
    cat ../$1.tar.gz* | tar xzf -
    if [ ${?} -ne 0 ];then
        log_error "### Decompress error for compressed file. Please check exitence of files ###"
    fi
    if [ $IS_LOG_ONLY_COMPRESSED == "0" ];then
        cat ../$1.$LOG_SUFFIX.tar.gz* | tar xzf -
        if [ ${?} -ne 0 ];then
            log_error "### Decompress error for log compressed file, Check If *.log.tar.gz exists ###"
        fi
    fi
fi

tar xzf $LIB_FILE

if [ $IS_LOG_ONLY_COMPRESSED == "1" ]; then
    echo "This Tar compressed file contains only log file"
else    
    rm -rf load.gdb

    if [ "$2" = "dumplog" ]; then
        echo "set solib-absolute-prefix ""../""$DUMP_EXTRACT_PATH" > load.gdb
        echo "core ""$DUMP_EXTRACT_PATH" >> load.gdb
        cat ../get_first_info_from_dump.gdb >> load.gdb
        sed -i 's/ibof_gdb.py/..\/ibof_gdb.py/g' load.gdb
        gdb ./poseidonos -x load.gdb -batch
        rm -rf load.gdb
    fi

    echo "set solib-absolute-prefix ""../""$DUMP_EXTRACT_PATH" > load.gdb
    echo "core ""$DUMP_EXTRACT_PATH" >> load.gdb
    gdb ./poseidonos -x load.gdb
fi


