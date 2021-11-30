#!/bin/bash
source ../config.sh
fio_parser="./fio_output_parser.py"
CLI="../../bin/poseidonos-cli"
ARRAYNAME=POSArray
print_result()
{
	sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:/tmp/fio.json .
	echo "      kIOPS    BW(MiB/s)"
	for i in `seq 1 $INIT1_VOL_CNT`
	do
		echo -n "volume""$i""  "
		${fio_parser} -v $i -g 0 -t $1 > full_result
		IOPS1=`awk '{print $1}' full_result`
		BW1=`awk '{print $2}' full_result`
		BW=$(expr ${BW1}*0.953674 | bc)
		echo ${IOPS1}" "${BW}
	done
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

	# multi volume (default) bringup
	echo "bring up start"
	sudo ../1_psd_bringup/2_bring_up.sh ${1}
}
start_and_bringup ./multi_volume_config.sh
source ./multi_volume_config.sh


let TOT_TIME=${RAMP_TIME}+${IO_RUN_TIME}
let INIT1_VOL_CNT=${VOLUME_CNT}

echo
sshpass -p "${INIT_1_PW}" scp ../2_fio_perf_test/${INIT1_FIO_SCRIPT_FILE} ../2_fio_perf_test/${INIT1_PARSE_RESULT_FILE} ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/
# basic performance and throttle with specific value.

echo "throttling for random write"
echo "rand_write for "$TOT_TIME "sec without throttling"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write

echo
echo "rand_write for "$TOT_TIME "sec with throttling (10MiB/s, volume 1-3)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol1 --maxbw 10
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol1

${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol2 --maxbw 10
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol2

${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol3 --maxbw 10
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol3

echo "rand_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write

echo
echo "rand_write for "$TOT_TIME "sec with throttling (20kIOPs, volume 4)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol4 --maxiops 20

echo "rand_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write

echo
echo "rand_write for "$TOT_TIME "sec with single volume reset (200MiB/s, volume 3)"
${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol3 --maxbw 0 --maxiops 0
${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol3

echo "rand_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write

echo
echo "rand_write for "$TOT_TIME "sec with volume (150MiB/s, all volumes)"

for i in `seq 1 $INIT1_VOL_CNT`
do
	${CLI} qos create --array-name ${ARRAYNAME} --volume-name vol${i} --maxbw 150 --maxiops 0
	${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol${i}
done
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write


#shutdown
sudo ../1_psd_bringup/4_shutdown_pos.sh

start_and_bringup ./multi_volume_config_dirty.sh
source ./multi_volume_config_dirty.sh

echo "Check qos after normal power off / on (150MiB/s maintained, all volumes)"

for i in `seq 1 $INIT1_VOL_CNT`
do
	${CLI} qos list --array-name ${ARRAYNAME} --volume-name vol${i}
done

sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write

echo
echo
echo "reset all volume's qos"
for i in `seq 1 $INIT1_VOL_CNT`
do
	${CLI} qos reset --array-name ${ARRAYNAME} --volume-name vol${i}
done
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 4k -q 128 -o 2 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -j -v -i ${TARGET_IP_1} > /dev/null 2>&1"
print_result write
sleep 2


