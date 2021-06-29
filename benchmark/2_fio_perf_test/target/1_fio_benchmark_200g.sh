#!/bin/bash
source ../../config.sh

let TOT_TIME=${RAMP_TIME}+${IO_RUN_TIME}
let INIT1_VOL_CNT=(${VOLUME_CNT}+1)/2
let INIT2_VOL_CNT=${VOLUME_CNT}-${INIT1_VOL_CNT}

echo "rand_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1 &"
sshpass -p "${INIT_2_PW}" ssh ${INIT_2_ID}@${INIT_2_IP} "cd ${INIT2_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT2_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 2 -b 4k -q 128 -c ${INIT2_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT2_FIO_ENGINE} -i ${TARGET_IP_2} > /dev/null 2>&1"
sleep 2
sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/4k_log/full_result .

echo "IOPS    BW"
echo "Initiator 1"
cat full_result
sshpass -p "${INIT_2_PW}" scp -r ${INIT_2_ID}@${INIT_2_IP}:${INIT2_FIO_SCRIPT_DIR}/4k_log/full_result .
echo "Initiator 2"
cat full_result

echo
echo "seq_write for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1 &"
sshpass -p "${INIT_2_PW}" ssh ${INIT_2_ID}@${INIT_2_IP} "cd ${INIT2_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT2_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 0 -b 128k -q 4 -c ${INIT2_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT2_FIO_ENGINE} -i ${TARGET_IP_2} > /dev/null 2>&1"
sleep 2
sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/128k_log/full_result .
echo "IOPS    BW"
echo "Initiator 1"
cat full_result
sshpass -p "${INIT_2_PW}" scp -r ${INIT_2_ID}@${INIT_2_IP}:${INIT2_FIO_SCRIPT_DIR}/128k_log/full_result .
echo "Initiator 2"
cat full_result

echo
echo "seq_read for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 1 -b 128k -q 4 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1 &"
sshpass -p "${INIT_2_PW}" ssh ${INIT_2_ID}@${INIT_2_IP} "cd ${INIT2_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT2_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 1 -b 128k -q 4 -c ${INIT2_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT2_FIO_ENGINE} -i ${TARGET_IP_2} > /dev/null 2>&1"
sleep 2
sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/128k_log/full_result .
echo "IOPS    BW"
echo "Initiator 1"
cat full_result
sshpass -p "${INIT_2_PW}" scp -r ${INIT_2_ID}@${INIT_2_IP}:${INIT2_FIO_SCRIPT_DIR}/128k_log/full_result .
echo "Initiator 2"
cat full_result

echo
echo "rand_read for "$TOT_TIME "sec"
sshpass -p "${INIT_1_PW}" ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT1_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 3 -b 4k -q 128 -c ${INIT1_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT1_FIO_ENGINE} -i ${TARGET_IP_1} > /dev/null 2>&1 &"
sshpass -p "${INIT_2_PW}" ssh ${INIT_2_ID}@${INIT_2_IP} "cd ${INIT2_FIO_SCRIPT_DIR}; echo ${INIT_1_PW} | sudo -S nohup ${INIT2_FIO_SCRIPT_DIR}/${INIT1_FIO_SCRIPT_FILE} -w 3 -b 4k -q 128 -c ${INIT2_VOL_CNT} -a ${RAMP_TIME} -r ${IO_RUN_TIME} -e ${INIT2_FIO_ENGINE} -i ${TARGET_IP_2} > /dev/null 2>&1"
sleep 2
sshpass -p "${INIT_1_PW}" scp -r ${INIT_1_ID}@${INIT_1_IP}:${INIT1_FIO_SCRIPT_DIR}/4k_log/full_result .
echo "IOPS    BW"
echo "Initiator 1"
cat full_result
sshpass -p "${INIT_2_PW}" scp -r ${INIT_2_ID}@${INIT_2_IP}:${INIT2_FIO_SCRIPT_DIR}/4k_log/full_result .
echo "Initiator 2"
cat full_result
