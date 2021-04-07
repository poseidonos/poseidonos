#!/bin/bash

logfile="ibofos.log"
rootdir=$(readlink -f $(dirname $0))/../../..
fiodir=${rootdir}/test/system/io_path
ip="10.100.11.24"
test_iteration=200
totalsize=100
volcnt=4
test_time=3600
sizepervol=`expr $totalsize / $volcnt `

# mfs file load mode [true, false]
mfsFileLoad="false"
# array mode [normal, degraded]
arraymode="normal"
# shutdown type [none, spor, npor]
shutdowntype="none"
# rebuild mode [none, rebuild_before_gc, rebuild_after_gc]
rebuild="none"

while getopts "f:p:a:" opt
do
    case "$opt" in
        f) ip="$OPTARG"
            ;;
        p) shutdowntype="$OPTARG"
            ;;
        a) arraymode="$OPTARG"
            ;;
    esac
done

res=0

shutdown()
{
	while :
	do
		ps -C ibofos > /dev/null
		if [[ ${?} != 0 ]]; then
			break;
		fi

		state=$(../bin/cli request info --json | jq -r '.Response.info.state' 2>/dev/null)
		if [[ $state = "NORMAL" ]]; then
			../bin/cli request unmount_ibofos
		elif [[ $state = "OFFLINE" ]]; then
			../bin/cli request exit_ibofos
		fi

		sleep 1s
	done
}

wait_user_input_y()
{
    while true
    do
        read -n1 -p "Press 'y' to continue next step..." input
        case ${input} in
            y) break
        esac
    done
    echo ""
}

check_result()
{
    if [ $res -ne 0 ];
    then
        banner failed
        wait_user_input_y
    else
        banner success
    fi
}

sudo ${rootdir}/test/script/kill_ibofos.sh
sudo ${rootdir}/script/start_ibofos.sh
sleep 10

sudo ./setup_ibofos.sh create ${arraymode} ${totalsize} ${volcnt} ${ip}

iotype="write"
#blocksize="512b-128k"
blocksize="128k"
sudo ${fiodir}/fio_bench.py --traddr=${ip} --trtype=tcp \
--readwrite=${iotype} --io_size=${sizepervol}G --verify=false \
--bs=${blocksize} --time_based=0 --run_time=0 --iodepth=32 \
--file_num=${volcnt}

res=$?
check_result

echo "prepare write done" >> mem_history.txt
sudo ./mem_check.sh >> mem_history.txt

if [ ${rebuild} == "rebuild_before_gc" ]; then
    rebuild=none
    sudo ${rootdir}/script/detach_device.sh unvme-ns-0 1
fi

for ((i=0;i<${test_iteration};i++))
do
    banner testcnt:${i}

    if [ $shutdowntype != "none" ]; then
        if [ $shutdowntype == "npor" ]; then
			shutdown
        else
            echo "add spor test"
        fi
        sleep 10

        sudo ${rootdir}/script/start_ibofos.sh
        sleep 10
        sudo ./setup_ibofos.sh load ${arraymode} ${totalsize} ${volcnt} ${ip}
    fi


    iotype="randwrite,write"
    sudo ${fiodir}/fio_bench.py --traddr=${ip} --trtype=tcp \
    --readwrite=${iotype} --io_size=${sizepervol}G --verify=true \
    --bs=${blocksize} --time_based=0 --run_time=0 --iodepth=32 \
    --file_num=${volcnt}
    
    res=$?
    check_result
    
    if [ ${rebuild} == "rebuild_after_gc" ]; then
        rebuild=none
        sudo ${rootdir}/script/detach_device.sh unvme-ns-0 1
    fi

    echo "io test conut : ${i}" >> mem_history.txt
    sudo ./mem_check.sh >> mem_history.txt
done

shutdown

echo "test end"
