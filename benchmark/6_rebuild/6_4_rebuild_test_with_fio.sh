#!/bin/bash

source ../config.sh

# Default Configuration
target_ip_1=${TARGET_IP_1}
target_ip_2=${TARGET_IP_2}
target_nic_1=${TARGET_NIC_1}
target_nic_2=${TARGET_NIC_2}
volume_cnt=${VOLUME_CNT}
volume_gb_size=${VOLUME_GB_SIZE}
volume_byte_size=${VOLUME_BYTE_SIZE}

init1_id=${INIT_1_ID}
init2_id=${INIT_2_ID}
init1_pw=${INIT_1_PW}
init2_pw=${INIT_2_PW}
init1_ip=${INIT_1_IP}
init2_ip=${INIT_2_IP}
init1_fio_conf_dir=${INIT1_FIO_CONF_DIR}
init2_fio_conf_dir=${INIT2_FIO_CONF_DIR}

ibofos_root="../.."
ibof_cli="${ibofos_root}/bin/cli"
ARRAYNAME=POSArray
# Test Configuration
sleep_time=10
predata_write_time=120
rebuild_io_time=600
rebuild_test_time=600000
need_bringup=true
need_predata_write=true
enable_io_during_rebuild=false
impact_level="high" # high | low

start_and_bringup()
{
	echo "pos start"
	sudo pkill -9 poseidonos
	sshpass -p ${init1_pw} ssh -tt ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S pkill -9 fio"
	sshpass -p ${init2_pw} ssh -tt ${init2_id}@${init2_ip} "echo ${init1_pw} | sudo -S pkill -9 fio"
	sudo ./6_1_start_pos.sh

	echo "bring up start"
	sudo ./6_2_bring_up.sh -a ${target_ip_1} -b ${target_ip_2} -s ${volume_cnt} -v ${volume_cnt} -S ${volume_byte_size} -n ${target_nic_1} -m ${target_nic_2}
}

waiting_for_rebuild_complete()
{
	echo "waiting for rebuild complete"
	wait_time=0
	while :
	do
		state=$(${ibof_cli} --json array info --name $ARRAYNAME | jq '.Response.result.data.state')
		if [ $state = "\"NORMAL\"" ]; then
			break;
		else
            rebuild_progress=$(${ibof_cli} --json array info --name $ARRAYNAME | jq '.Response.result.data.rebuildingProgress')
            echo "Rebuilding Progress [${rebuild_progress}]"
			sleep ${sleep_time}
			wait_time=$((${wait_time} + ${sleep_time}))
		fi

		if [ "${wait_time}" -gt "${rebuild_test_time}" ]; then
			break;
		fi
	done

}

calc_performance()
{
	mkdir -p rebuild_fio_result_init_1
	mkdir -p rebuild_fio_result_init_2
	sshpass -p ${init1_pw} scp -o StrictHostKeyChecking=no ${init1_id}@${init1_ip}:${init1_fio_conf_dir}/*.log ./rebuild_fio_result_init_1/
	sshpass -p ${init2_pw} scp -o StrictHostKeyChecking=no ${init2_id}@${init2_ip}:${init2_fio_conf_dir}/*.log ./rebuild_fio_result_init_2/
	sshpass -p ${init1_pw} ssh ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S rm ${init1_fio_conf_dir}/*.log"
	sshpass -p ${init2_pw} ssh ${init2_id}@${init2_ip} "echo ${init2_pw} | sudo -S rm ${init2_fio_conf_dir}/*.log"
	sudo ./calculate_file.py

	echo "performance result"
	cat rebuild_rand_result.txt
}

if [ $need_bringup = true ]; then
	start_and_bringup
fi

if [ $need_predata_write = true ]; then
	echo "write pre data"
	sudo ./6_3_run_fio.sh -r ${predata_write_time}
	echo "write done"
	calc_performance
fi

echo "setting rebuild performance impact ${impact_level} by cli"
sudo ${ibof_cli} rebuild perf_impact --level ${impact_level}
echo "rebuild test start"
if [ $enable_io_during_rebuild = true ]; then
	sudo ./6_3_run_fio.sh -r ${rebuild_io_time} &
fi
echo sleep
sleep 15
sudo ${ibofos_root}/test/script/detach_device.sh unvme-ns-0 1
waiting_for_rebuild_complete

echo "rebuild test finished"

calc_performance
