#!/bin/bash

TARGET_DIR=$1
ST_UNKNOWN=0
ST_FIND=1
ST_OKAY=2
ST_NEED=3
STATUS=$ST_UNKNOWN

while read LINE; do
    if [[ "$LINE" == *"$TARGET_DIR"* ]];then
        echo $LINE
        STATUS=$ST_FIND
        continue
    fi
    if [ $STATUS -eq $ST_FIND ];then
        if [[ "$LINE" == *"100.00%"* ]];then
            STATUS=$ST_OKAY
            echo -e "\033[32m"$LINE"\033[0m"
        else
            STATUS=$ST_NEED
            echo -e "\033[31m"$LINE"\033[0m"
        fi
        continue
    fi
    if [[ "$LINE" == *"Creating"* ]];then
        if [ $STATUS -ne $ST_NEED ];then
            FILE_LEN=${#LINE}-11
            FILE=${LINE:10:FILE_LEN}
            rm -rf $FILE
        fi
        STATUS=$ST_UNKNOWN
        continue
    fi
done < result.txt
