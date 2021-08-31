#!/bin/bash

source ../config.sh

init1_id=${INIT_1_ID}
init2_id=${INIT_2_ID}
init1_pw=${INIT_1_PW}
init2_pw=${INIT_2_PW}
init1_ip=${INIT_1_IP}
init2_ip=${INIT_2_IP}
volume_cnt=${VOLUME_CNT}

disconnect_nvme()
{
for ((i=1;i<=${volume_cnt};i+=2))
do
    echo "init1 disconnect $i"
    sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
done

for ((i=2;i<=${volume_cnt};i+=2))
do
    echo "init2 disconnect $i"
    sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
done

}

kill_vdbench()
{
    sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S pkill -9 vdbench; echo ${init1_pw} | sudo -S pkill -9 java"
    sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S pkill -9 vdbench; echo ${init2_pw} | sudo -S pkill -9 java"
}

disconnect_nvme
kill_vdbench
