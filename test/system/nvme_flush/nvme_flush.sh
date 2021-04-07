#!/bin/bash
totalFlushFailCount=0

flushSuccessCount=0
#TODO: Change seq length based on write io size
for i in $(seq 1 100)
do
    nvme flush $1 -n 1 2> /dev/null | grep -q 'success' && flushSuccessCount=$((flushSuccessCount+1))
done
totalFlushFailCount=$((100-flushSuccessCount+totalFlushFailCount))

echo $totCount
