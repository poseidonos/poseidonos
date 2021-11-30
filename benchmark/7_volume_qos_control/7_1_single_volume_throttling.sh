#!/bin/bash
source ../config.sh
reactor_count=${VOLUME_CNT}
CLI="../../bin/poseidonos-cli"
ARRAYNAME=POSArray
print_result()
{
	sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/${1}/full_result .
	echo "              kIOPS    BW(MiB/s)"
	echo -n "Initiator 1 : "
	IOPS1=`awk '{print $1}' full_result`
	BW1=`awk '{print $2}' full_result`
    BW=$(expr ${BW1}*0.953674 | bc)
	echo ${IOPS1}" "${BW}
}

start_and_bringup()
{
	echo "pos start"
	sudo pkill -9 poseidonos
	sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "echo ${INIT_1_PW} | sudo -S pkill -9 fio"
	echo "clean initiator complete"
	sudo cp ../1_psd_bringup/pos.conf /etc/pos/
	jq -r '.fe_qos.enable = true' /etc/pos/pos.conf > /tmp/temp.json && mv /tmp/temp.json /etc/pos/pos.conf
	jq -r '.fe_qos.user_initiator = true' /etc/pos/pos.conf > /tmp/temp.json && mv /tmp/temp.json /etc/pos/pos.conf

	sudo ../1_psd_bringup/1_start_pos.sh

	echo "bring up start"
	sudo ../1_psd_bringup/2_bring_up.sh ./single_volume_config.sh
}


start_and_bringup
source ./single_volume_config.sh
let TOT_TIME=${RAMP_TIME}+${IO_RUN_TIME}
let INIT1_VOL_CNT=${VOLUME_CNT}

echo
sshpass -p "${INIT_1_PW}" scp ../2_fio_perf_test/${INIT1_FIO_SCRIPT_FILE} ../2_fio_perf_test/${INIT1_PARSE_RESULT_FILE} ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/
# basic performance and throttle with specific value.


echo "1. throttling for sequential write"
echo "seq_write for "$TOT_TIME "sec without throttling"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
sleep 2
print_result "128k_log"

echo
echo "seq_write for "$TOT_TIME "sec with throttling (10000MiB/s)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxbw 10000
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
sleep 2
print_result "128k_log"

echo
echo "seq_write for "$TOT_TIME "sec with throttling (1000MiB/s)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxbw 1000
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
sleep 2
print_result "128k_log"

echo
echo "seq_write for "$TOT_TIME "sec with throttling reset"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxbw 0
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
sleep 2
print_result "128k_log"

echo
echo
echo
echo "2. Throttling on run-time for random write"
echo "rand_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result "4k_log"

echo
echo "rand_write for "$TOT_TIME "sec with throttling (1000MiB/s)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxbw 1000
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result "4k_log"

# 70k iops is set
echo
echo "rand_write for "$TOT_TIME "sec with throttling (1000MiB/s, 70kiops), 50kiops will throttle 4k io"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxiops 70
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result "4k_log"

# 400k iops is set
echo
echo "rand_write for "$TOT_TIME "sec with throttling (1000MiB/s, 400kiops), 1000MiB/s will throttle 4k io"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxiops 400
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -o 1 -c ${INIT1_VOL_CNT} -n ${reactor_count} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result "4k_log"

 
# initialize config again.
sudo cp ../1_psd_bringup/pos.conf /etc/pos/

