#!/bin/bash
#TODO: Change seq length based on write io size
failedSmartLogCount=0
fieldType=$2
filePath=$3
outputFile=OUTPUT
scriptFile=parse_smart_output.py
outputFilePath=$filePath/$outputFile
scriptFilePath=$filePath/$scriptFile
#need to change the for loop number
for i in $(seq 1 1)
do
    nvme smart-log $1 -n 1 -o json > $outputFilePath
    res=$(python $scriptFilePath $fieldType $outputFilePath)
    if [ ${res} -eq 0 ]; then
        failedSmartLogCount=$((failedSmartLogCount+1))
    fi

done
echo $failedSmartLogCount
