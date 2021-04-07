#!/bin/bash

# change working directory to where script exists
cd $(dirname $0)
dirpath=`dirname $0`
rootdir=$(readlink -f ${dirpath})/../../..

sudo rm ${dirpath}/test_done.txt
sudo ${rootdir}/test/script/kill_ibofos.sh 

cd ${rootdir};

sudo make -j 12                                                                 

if [ $? -ne 0 ]                                                                 
then                                                                            
sudo ./script/build_ibofos.sh >> /dev/null                                      
fi 

cd -;

sudo ${rootdir}/script/start_ibofos.sh

procid=`pgrep ibofos`
echo "get pgrep $procid"

sudo ${dirpath}/bisect_longterm_test.sh & >> /dev/null

while [ ! -f "${dirpath}/test_done.txt" ]
do
    sudo pgrep ibofos >> /dev/null
    if [ $? -ne 0 ];
    then
        sudo pkill -9 fio
        exit 1
    fi

    sleep 30
done

exit 0

