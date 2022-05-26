#!/bin/bash

rootdir=$(readlink -f $(dirname $0))/../..
bindir=$rootdir/bin

NUM_OF_TESTS=1000

./kill_poseidonos.sh
sudo pkill -9 poseidonos-cli

$bindir/poseidonos-cli system start

for ((i = 0; i < $NUM_OF_TESTS; i++))
do
    nohup $bindir/poseidonos-cli system set-property --rebuild-impact medium --json-res &
    nohup $bindir/poseidonos-cli system get-property --json-res &
    nohup $bindir/poseidonos-cli system set-property --rebuild-impact highest --json-res &
    nohup $bindir/poseidonos-cli system get-property --json-res &
    nohup $bindir/poseidonos-cli system get-property --json-res &
    nohup $bindir/poseidonos-cli system set-property --rebuild-impact false_value --json-res &
    $bindir/poseidonos-cli system info --json-res
done

num_of_empty_response=$(grep -o '{}' nohup.out | wc -l)

if [ $num_of_empty_response -eq 0 ]
then
    echo 'PASS' 
else
    echo 'FAIL'
fi

echo 'The number of errorneous response due to concurrent commands: ' $num_of_empty_response

rm ./nohup.out