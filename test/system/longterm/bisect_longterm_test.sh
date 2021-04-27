#!/bin/bash
dirpath=`dirname $0`
echo $dirpath

logfile="pos.log"

rootdir=$(readlink -f ${dirpath})/../../..
fiodir=${dirpath}/../nvmf/initiator
ip="10.100.11.24"
totalsize=110
volcnt=1
sizepervol=`expr $totalsize / $volcnt `

# array mode [normal, degraded]
arraymode="normal"
# shutdown type [none, spor, npor]
shutdowntype="none"

res=0
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

sudo ${dirpath}/setup_ibofos.sh create ${arraymode} ${totalsize} ${volcnt} ${ip}

iotype="write"
#blocksize="512b-128k"
blocksize="128k"

sudo ${fiodir}/fio_full_bench.py --traddr=${ip} --trtype=tcp --readwrite=${iotype} --io_size=${sizepervol}G --verify=false --bs=${blocksize} --time_based=1 --run_time=300 --iodepth=32 --file_num=${volcnt}
res=$?
check_result

echo "test done" >> ${rootdir}/test_done.txt
sudo ${dirpath}/mem_check.sh >> ${rootdir}/test_done.txt

for ((i=0;i<2;i++))
do

    banner testcnt:${i}

    if [ $shutdowntype != "none" ]; then
        if [ $shutdowntype == "npor" ]; then
            sudo ${rootdir}/bin/cli system exit
        else
            echo "add spor test"
        fi
        sleep 10

        sudo ${rootdir}/script/start_poseidonos.sh
        sleep 10
        sudo ./setup_ibofos.sh load ${arraymode} ${totalsize} ${volcnt} ${ip}
    fi


    iotype="randwrite"
    blocksize="4k"
    sudo ${fiodir}/fio_full_bench.py --traddr=${ip} --trtype=tcp --readwrite=${iotype} --io_size=${sizepervol}G --verify=true --bs=${blocksize} --time_based=1 --run_time=300 --iodepth=32 --file_num=${volcnt}
    res=$?
    check_result

    echo "io test conut : ${i}" >> mem_history.txt
    sudo ./mem_check.sh >> mem_history.txt
done

sudo ${rootdir}/test/script/kill_poseidonos.sh
echo "test end"
