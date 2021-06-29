#!/bin/bash
source ../config.sh

root_dir=${ROOT_DIR}
target_ip_1=${TARGET_IP_1}
target_ip_2=${TARGET_IP_2}
target_nic_1=${TARGET_NIC_1}
target_nic_2=${TARGET_NIC_2}
volume_cnt=${VOLUME_CNT}
volume_gb_size=${VOLUME_GB_SIZE}
volume_byte_size=${VOLUME_BYTE_SIZE}
# second
write_fill_seq_io_time=${WRITE_FILL_SEQ_IO_TIME}
write_fill_rand_io_time=${WRITE_FILL_RAND_IO_TIME}
seq_io_time=${VOL_DEL_SEQ_IO_TIME}
rand_io_time=${VOL_DEL_RAND_IO_TIME}
delete_volume_cnt=${DELETE_VOLUME_CNT}
remain_volume_cnt=${REMAIN_VOLUME_CNT}
deleted_start_volume_num=$((remain_volume_cnt+1))

init1_id=${INIT_1_ID}
init2_id=${INIT_2_ID}
init1_pw=${INIT_1_PW}
init2_pw=${INIT_2_PW}
init1_ip=${INIT_1_IP}
init2_ip=${INIT_2_IP}
init1_vdbench_dir=${INIT1_VDBENCH_DIR}
init2_vdbench_dir=${INIT2_VDBENCH_DIR}
vdbench_sub_initiator_ip=${VDBENCH_SUB_INIT_IP}

volume_delete()
{
for ((i=${deleted_start_volume_num};i<=${volume_cnt};i++))
do
    if [ `expr ${i} % 2` -eq 1 ]
    then
        echo "init1 disconnect $i"
 #       sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
    else
        echo "init2 disconnect $i"
 #       sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
    fi
done


    for ((i=${deleted_start_volume_num};i<=${volume_cnt};i++))
    do
        $root_dir/bin/cli volume unmount --name vol$i --array POSArray
        $root_dir/bin/cli volume delete --name vol$i --array POSArray
    done
}

volume_create_and_mount()
{
for ((i=${deleted_start_volume_num};i<=${volume_cnt};i++))
do
    sudo $root_dir/bin/cli volume create --name vol$i --size $volume_byte_size --maxiops 0 --maxbw 0 --array POSArray
    sudo $root_dir/bin/cli volume mount --name vol$i --array POSArray
done
for ((i=${deleted_start_volume_num};i<=${volume_cnt};i++))
do
    if [ `expr ${i} % 2` -eq 1 ]
    then
        echo "init1 connect $i"
      #  sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_1} -s 1158"
    else
        echo "init2 connect $i"
      #  sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_2} -s 1158"
    fi
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

connect_nvme()
{
for ((i=1;i<=${volume_cnt};i++))
do
    if [ `expr ${i} % 2` -eq 1 ]
    then
        echo "init1 connect $i"
        sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_1} -s 1158"
    else
        echo "init2 connect $i"
        sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme connect -t tcp -n nqn.2019-04.pos:subsystem${i} -a ${target_ip_2} -s 1158"
    fi
done
echo "connect finish"
}

disconnect_nvme()
{
for ((i=1;i<=${volume_cnt};i++))
do
    if [ `expr ${i} % 2` -eq 1 ]
    then
        echo "init1 disconnect $i"
        sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
    else
        echo "init2 disconnect $i"
        sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme disconnect -n nqn.2019-04.pos:subsystem${i}"
    fi
done

echo "disconnect finish"
}

write_fill_pos_before_vol_delete()
{
echo "write fill pos"
echo "get nvme list"
nvmelist1=`sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme list" | awk '/dev/{print $1}' | awk '{printf("%s ", $0)}'`
nvmelist2=`sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme list" | awk '/dev/{print $1}' | awk '{printf("%s ", $0)}'`

echo nvme list : ${nvmelisti1}
echo nvme list : ${nvmelisti2}

echo "write fill pos"
sudo ./create_test_config.sh -a ${target_ip_1} -b ${target_ip_2} -v ${volume_cnt} -S ${volume_gb_size} -s ${write_fill_seq_io_time} -r ${write_fill_rand_io_time} -p ${vdbench_sub_initiator_ip} -d ${init1_vdbench_dir} -l "${nvmelist1}" -L "${nvmelist2}"

sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "cd ${init1_vdbench_dir}; echo ${init1_pw} | sudo -S nohup ./vdbench rsh > /dev/null 2>&1 &"

sshpass -p ${init2_pw} scp -o StrictHostKeyChecking=no ./longterm_test.vd ${init2_id}@${init2_ip}:${init2_vdbench_dir}
sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "cd ${init2_vdbench_dir}; echo ${init2_pw} | sudo -S ./vdbench -f ./longterm_test.vd -o write_fill"
}

longterm_test_after_vol_delete()
{
echo "vol delete"
volume_delete

echo "get nvme list"
nvmelist1=`sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S nvme list" | awk '/dev/{print $1}' | awk '{printf("%s ", $0)}'`
nvmelist2=`sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S nvme list" | awk '/dev/{print $1}' | awk '{printf("%s ", $0)}'`
echo nvme list : $nvmelisti1
echo nvme list : $nvmelisti2

echo "longterm test start"
sudo ./create_test_config.sh -a ${target_ip_1} -b ${target_ip_2} -v ${remain_volume_cnt} -S ${volume_gb_size} -s ${seq_io_time} -r ${rand_io_time} -p ${vdbench_sub_initiator_ip} -d ${init1_vdbench_dir} -l "${nvmelist1}" -L "${nvmelist2}"

sshpass -p ${init2_pw} scp -o StrictHostKeyChecking=no ./longterm_test.vd ${init2_id}@${init2_ip}:${init2_vdbench_dir}
sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "cd ${init2_vdbench_dir}; echo ${init2_pw} | sudo -S ./vdbench -f ./longterm_test.vd -o longterm_test_after_volume_delete"
volume_create_and_mount
echo "longterm test finish"
}

echo "test start"
sudo ../1_psd_bringup/1_start_pos.sh
sudo ../1_psd_bringup/2_bring_up.sh -a ${target_ip_1} -b ${target_ip_2} -s ${volume_cnt} -v ${volume_cnt} -S ${volume_byte_size} -n ${target_nic_1} -m ${target_nic_2}

#copy_and_unzip_vdbench

connect_nvme

write_fill_pos_before_vol_delete

longterm_test_after_vol_delete

disconnect_nvme


