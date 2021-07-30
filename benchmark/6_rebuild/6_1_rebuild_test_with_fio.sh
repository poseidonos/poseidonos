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
sleep_time=5
predata_write_time=${REBUILD_PREDATA_WRITE_TIME}
rebuild_io_time=${REBUILD_IO_DURING_REBUILD_TIME}
rebuild_test_time=${REBUILD_MAX_REBUILD_TEST_TIME}
need_bringup=${REBUILD_NEED_BRINGUP}
need_predata_write=${REBUILD_NEED_PREDATA_WRITE}
enable_io_during_rebuild=${REBUILD_NEED_IO_DURING_REBUILD}
impact_level=${REBUILD_IMPACT}

start_and_bringup()
{
	echo "pos start"
	sudo pkill -9 poseidonos
	sshpass -p ${init1_pw} ssh -q -tt ${init1_id}@${init1_ip} "echo ${init1_pw} | sudo -S pkill -9 fio"
	sshpass -p ${init2_pw} ssh -q -tt ${init2_id}@${init2_ip} "echo ${init1_pw} | sudo -S pkill -9 fio"
	sudo ../1_psd_bringup/1_start_pos.sh

	echo "bring up start"
	sudo ../1_psd_bringup/2_bring_up.sh
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

if [ $need_bringup = true ]; then
	start_and_bringup
fi

if [ $need_predata_write = true ]; then
	echo "write pre data"
	sudo ./run_fio.sh -r ${predata_write_time}
	echo "write done"
fi

echo "setting rebuild performance impact ${impact_level} by cli"
sudo ${ibof_cli} rebuild perf_impact --level ${impact_level}
echo "rebuild test start"
if [ $enable_io_during_rebuild = true ]; then
	sudo ./run_fio.sh -r ${rebuild_io_time} &
fi
echo sleep
sleep 15
echo ""
echo "Device Detach"
sudo ${ibofos_root}/test/script/detach_device.sh unvme-ns-0 1
waiting_for_rebuild_complete

echo "rebuild test finished"

