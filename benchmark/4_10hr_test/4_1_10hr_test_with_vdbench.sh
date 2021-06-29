#!/bin/bash

source ../config.sh

target_ip_1=${TARGET_IP_1}
target_ip_2=${TARGET_IP_2}
target_nic_1=${TARGET_NIC_1}
target_nic_2=${TARGET_NIC_2}
volume_cnt=${VOLUME_CNT}
volume_gb_size=${VOLUME_GB_SIZE}
volume_byte_size=${VOLUME_BYTE_SIZE}
# second
seq_io_time=${LONGTERM_SEQ_IO_TIME}
rand_io_time=${LONGTERM_RAND_IO_TIME}

init1_id=${INIT_1_ID}
init2_id=${INIT_2_ID}
init1_pw=${INIT_1_PW}
init2_pw=${INIT_2_PW}
init1_ip=${INIT_1_IP}
init2_ip=${INIT_2_IP}
init1_vdbench_dir=${INIT1_VDBENCH_DIR}
init2_vdbench_dir=${INIT2_VDBENCH_DIR}
vdbench_sub_initiator_ip=${VDBENCH_SUB_INIT_IP}

connect_nvme()
{
for ((i=1;i<=${volume_cnt};i+=2))
do
    echo "init1 connect $i"
    sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_1} -s 1158"
done

echo "prepare init 1 finish"
for ((i=2;i<=${volume_cnt};i+=2))
do
    echo "init2 connect $i"
    sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_2} -s 1158"
done
echo "prepare init 2 finish"
}

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

copy_and_unzip_vdbench()
{
sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "mkdir -p ${init1_vdbench_dir}"
sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "mkdir -p ${init2_vdbench_dir}"
sshpass -p ${init1_pw} scp -o StrictHostKeyChecking=no ./vdbench50407.zip ${init1_id}@${init1_ip}:${init1_vdbench_dir}
sshpass -p ${init2_pw} scp -o StrictHostKeyChecking=no ./vdbench50407.zip ${init2_id}@${init2_ip}:${init2_vdbench_dir}
sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "cd ${init1_vdbench_dir}; echo ${init1_pw} | sudo -S unzip ./vdbench50407.zip"
sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "cd ${init2_vdbench_dir}; echo ${init2_pw} | sudo -S unzip ./vdbench50407.zip"
}

run_vdbench()
{
echo "get nvme list"
nvmelist1=`sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme list" | awk '/dev/{print $1}'`
nvmelist2=`sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme list" | awk '/dev/{print $1}'`
echo nvme list : $nvmelisti1
echo nvme list : $nvmelisti2
sudo ./create_test_config.sh -a ${target_ip_1} -b ${target_ip_2} -v ${volume_cnt} -S ${volume_gb_size} -s ${seq_io_time} -r ${rand_io_time} -p ${vdbench_sub_initiator_ip} -d ${init1_vdbench_dir} -l "${nvmelist1}" -L "${nvmelist2}"

sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "cd ${init1_vdbench_dir}; echo ${init1_pw} | sudo -S nohup ./vdbench rsh > /dev/null 2>&1 &"

sshpass -p ${init2_pw} scp -o StrictHostKeyChecking=no ./longterm_test.vd ${init2_id}@${init2_ip}:${init2_vdbench_dir}
sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "cd ${init2_vdbench_dir}; echo ${init2_pw} | sudo -S ./vdbench -f ./longterm_test.vd -o longterm_test"
}

echo "pos start"
sudo ../1_psd_bringup/1_start_pos.sh

echo "bring up start"
sudo ../1_psd_bringup/2_bring_up.sh -a ${target_ip_1} -b ${target_ip_2} -s ${volume_cnt} -v ${volume_cnt} -S ${volume_byte_size} -n ${target_nic_1} -m ${target_nic_2}

connect_nvme
#copy_and_unzip_vdbench

echo "test start"
run_vdbench
disconnect_nvme

echo "longterm test finish"

