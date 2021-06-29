#!/bin/bash
source ../config.sh

vdfile=test_1ms.vd

### 1. Connect
turn=0
for i in `seq 1 $VOLUME_CNT`
do
    if [ $turn -eq 0 ]; then
	sshpass -p ${INIT_1_PW} ssh ${INIT_1_ID}@${INIT_1_IP} "echo ${INIT_1_PW} | sudo -S nvme connect -n nqn.2019-04.ibof:subsystem${i} -t tcp -a ${TARGET_IP_1} -s 1158"
        turn=1
    else
	sshpass -p ${INIT_2_PW} ssh ${INIT_2_ID}@${INIT_2_IP} "echo ${INIT_2_PW} | sudo -S nvme connect -n nqn.2019-04.ibof:subsystem${i} -t tcp -a ${TARGET_IP_2} -s 1158"
        turn=0
    fi
done
sleep 2

### 2. Get device list
devlist_init1=`sshpass -p ${INIT_1_PW} ssh ${INIT_1_ID}@${INIT_1_IP} "echo ${INIT_1_PW} | sudo -S nvme list | grep IBOF" | awk '{print $1}'`
devlist_init2=`sshpass -p ${INIT_2_PW} ssh ${INIT_2_ID}@${INIT_2_IP} "echo ${INIT_2_PW} | sudo -S nvme list | grep IBOF" | awk '{print $1}'`

### 3. Make vdbench file
count_init1=0
count_init2=0
echo hd=default,jvms=20 > ${vdfile}
echo hd=localhost >> ${vdfile}
echo hd=remote,shell=vdbench,system=${INIT_2_IP},vdbench=${INIT2_VDBENCH_DIR},user=${INIT_2_ID} >> ${vdfile}

for diskname in ${devlist_init1[@]}
do
    echo sd=nvme00${count_init1},host=localhost,lun=${diskname},openflags=o_direct,size=8g >> ${vdfile}
    count_init1=$((count_init1+1))
done

for diskname in ${devlist_init2[@]}
do
    echo sd=nvme10${count_init2},host=remote,lun=${diskname},openflags=o_direct,size=8g >> ${vdfile}
    count_init2=$((count_init2+1))
done

echo "wd=seq,sd=nvme*,xfersize=4k,rdpct=0,seekpct=0" >> ${vdfile}
echo "wd=rand,sd=nvme*,xfersize=4k,rdpct=0,seekpct=100" >> ${vdfile}

if [ ${DEMO_TEST} -eq 1 ]; then
	echo "rd=seq_r,wd=seq,iorate=max,elapsed=15,interval=3,warmup=3,pause=5,forxfersize=(4k),forrdpct=(100),forthreads=(1,64)" >> ${vdfile}
	echo "rd=rand_r,wd=rand,iorate=max,elapsed=15,interval=3,warmup=3,pause=5,forxfersize=(4k),forrdpct=(100),forthreads=(1,64)" >> ${vdfile}
else
	echo "rd=seq_r,wd=seq,iorate=max,elapsed=15,interval=3,warmup=3,pause=5,forxfersize=(4k,8k,16k,128k,256k,512k,1m,4m),forrdpct=(100,70,50,30,0),forthreads=(1,2,4,8,16,32,64,128)" >> ${vdfile}
	echo "rd=rand_r,wd=rand,iorate=max,elapsed=15,interval=3,warmup=3,pause=5,forxfersize=(4k,8k,16k,128k,256k,512k,1m,4m),forrdpct=(100,70,50,30,0),forthreads=(1,2,4,8,16,32,64,128)" >> ${vdfile}
fi

### 4. Copy vdbench script
sudo sshpass -p ${INIT_1_PW} scp ./${vdfile} ${INIT_1_ID}@${INIT_1_IP}:${INIT1_VDBENCH_DIR}/


### 5. Run vdbench
sshpass -p ${INIT_2_PW} ssh ${INIT_2_ID}@${INIT_2_IP} "echo ${INIT_2_PW} | sudo -S ${INIT2_VDBENCH_DIR}/vdbench rsh" &
sleep 1
sshpass -p ${INIT_1_PW} ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_VDBENCH_DIR}; echo ${INIT_1_PW} | sudo -S ./vdbench -f ./${vdfile} -o result_1ms"
sshpass -p ${INIT_1_PW} ssh ${INIT_1_ID}@${INIT_1_IP} "cd ${INIT1_VDBENCH_DIR}; echo ${INIT_1_PW} | sudo -S ./vdbench parseflat -i ./result_1ms/flatfile.html -a -c rate MB/sec read% resp read_resp write_resp resp_max read_max write_max xfersize threads rdpct rhpct whpct seekpct lunsize queue_depth -o rawdata_1ms.csv"
pkill -9 sshpass


### 6. Copy result file
sudo sshpass -p ${INIT_1_PW} scp ${INIT_1_ID}@${INIT_1_IP}:${INIT1_VDBENCH_DIR}/rawdata_1ms.csv ./


### 8. Disconnect
turn=0
for i in `seq 1 $VOLUME_CNT`
do
    if [ $turn -eq 0 ]; then
	sshpass -p ${INIT_1_PW} ssh ${INIT_1_ID}@${INIT_1_IP} "echo ${INIT_1_PW} | sudo -S nvme disconnect -n nqn.2019-04.ibof:subsystem${i}"
        turn=1
    else
	sshpass -p ${INIT_2_PW} ssh ${INIT_2_ID}@${INIT_2_IP} "echo ${INIT_2_PW} | sudo -S nvme disconnect -n nqn.2019-04.ibof:subsystem${i}"
        turn=0
    fi
done

