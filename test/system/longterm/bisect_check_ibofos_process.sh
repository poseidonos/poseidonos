#!/bin/bash

# change working directory to where script exists
cd $(dirname $0)
dirpath=`dirname $0`
rootdir=$(readlink -f ${dirpath})/../../..

sudo rm ${dirpath}/test_done.txt
sudo ${rootdir}/test/script/kill_poseidonos.sh 

cd ${rootdir};

sudo make -j 4

if [ $? -ne 0 ]
then
sudo ./script/build_ibofos.sh internal >> /dev/null
fi 

cd -;

sudo ${rootdir}/script/start_poseidonos.sh

procid=`pgrep poseidonos`
echo "get pgrep $procid"

sudo ${rootpath}/test/regression/long_term_ci_test.sh & >> /dev/null

while [ ! -f "${dirpath}/test_done.txt" ]
do
    sudo pgrep poseidonos >> /dev/null
    if [ $? -ne 0 ];
    then
        sudo pkill -9 fio
        exit 1
    fi

    sleep 30
done

exit 0

