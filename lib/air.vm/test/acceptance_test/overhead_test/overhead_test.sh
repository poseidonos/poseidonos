#!/bin/bash

IBOFOS_ROOT_DIR=$(readlink -f $(dirname $0))/../../../../..
CUR_DIR=$(readlink -f $(dirname $0))
AIR_DIR=$(readlink -f $(dirname $0))/../../..
TARGET_IP_ADDR=0
CORE=11
FILE_WO_AIR=$(readlink -f $(dirname $0))/fio_result_wo_air.txt
FILE_W_AIR=$(readlink -f $(dirname $0))/fio_result_w_air.txt
FILE_W_AIR_PERF=$(readlink -f $(dirname $0))/fio_result_w_air_perf.txt
FILE_W_AIR_LAT=$(readlink -f $(dirname $0))/fio_result_w_air_lat.txt
FILE_RESULT=$(readlink -f $(dirname $0))/overhead_test_result.txt
FILE_XML=$(readlink -f $(dirname $0))/overhead_test_result.xml
PATCH_W_AIR=$(readlink -f $(dirname $0))/w_air.patch

TESTS=0
FAILURES=0

red_echo()   { echo -n -e "\e[1;31;40m $1 \e[0m"; }
green_echo() { echo -n -e "\e[1;32;40m $1 \e[0m"; }
blue_echo()  { echo -n -e "\e[1;36;40m $1 \e[0m"; }

function f_usage
{
	red_echo " Usage: sh $0 [ ip addr ] "; echo " "
	exit	
}

function f_evaluate 
{
    if [ "$1" == "wo_air" ] ; then
        cd ${AIR_DIR}/config
        mv air.cfg air_back.cfg
        cp at_ov_test.cfg air.cfg
        
        f_build_ibofos
        for i in {1..5}
        do
            f_start_ibofos 
            f_run_fio >> ${FILE_WO_AIR}
            f_kill_ibofos
        done
        cd ${AIR_DIR}/config
        mv air_back.cfg air.cfg

    elif [ "$1" == "w_air_perf" ] ; then
        cd ${IBOFOS_ROOT_DIR}
        patch -p0 < ${PATCH_W_AIR}
        cd ${AIR_DIR}/config
        mv air.cfg air_back.cfg
        cp at_perf_ov_test.cfg air.cfg

        f_build_ibofos
        for i in {1..5}
        do
            f_start_ibofos 
            f_run_fio >> ${FILE_W_AIR}
            f_kill_ibofos
        done
        cd ${AIR_DIR}/config
        mv air_back.cfg air.cfg
        cd ${IBOFOS_ROOT_DIR}
        patch -R -p0 < ${PATCH_W_AIR}
    
    elif [ "$1" == "w_air_lat" ] ; then
        cd ${IBOFOS_ROOT_DIR}
        patch -p0 < ${PATCH_W_AIR}
        cd ${AIR_DIR}/config
        mv air.cfg air_back.cfg
        cp at_lat_ov_test.cfg air.cfg

        f_build_ibofos
        for i in {1..5}
        do
            f_start_ibofos 
            f_run_fio >> ${FILE_W_AIR}
            f_kill_ibofos
        done
        cd ${AIR_DIR}/config
        mv air_back.cfg air.cfg
        cd ${IBOFOS_ROOT_DIR}
        patch -R -p0 < ${PATCH_W_AIR}
    else
        return
    fi
}

function f_build_ibofos
{   
    cd ${IBOFOS_ROOT_DIR}/script/
    ./build_ibofos.sh
}

function f_start_ibofos
{
    cd ${IBOFOS_ROOT_DIR}/script/
    ./start_poseidonos.sh
    sleep 2
    # setup nvmf vol0
    ../lib/spdk-19.10/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem1 -a -s IBOF00000000000001 -d IBOF_VOLUME_EXTENTION
    ../lib/spdk-19.10/scripts/rpc.py bdev_malloc_create -b uram0 1024 512
    ../bin/cli_client scan_dev
    ../bin/cli_client create_array -b 1 uram0 -d 3 unvme-ns-0 unvme-ns-1 unvme-ns-2 -s 1 unvme-ns-3
    ../bin/cli_client mount_ibofos
    ../bin/cli_client create_vol --name vol0 --size 2147483648 --maxiops 0 --maxbw 0
    ../bin/cli_client mount_vol --name vol0
    ../lib/spdk-19.10/scripts/rpc.py nvmf_create_transport -t TCP -u 131072 -p 4 -c 0
    ../lib/spdk-19.10/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem1 -t TCP -a ${TARGET_IP_ADDR} -s 1158
    ../lib/spdk-19.10/scripts/rpc.py get_nvmf_subsystems
    sleep 2
}

function f_kill_ibofos
{
    cd ${IBOFOS_ROOT_DIR}/script/
    ./shutdown.sh
}

function f_run_fio
{
    local filename='trtype=tcp adrfam=IPv4 traddr='${TARGET_IP_ADDR}' trsvcid=1158 ns=1'
    local fio_common_parameter="--thread=1 --group_reporting=1 --direct=1  --time_based=1 --cpus_allowed=${CORE} --ioengine=${IBOFOS_ROOT_DIR}/lib/spdk-19.10/examples/nvme/fio_plugin/fio_plugin --runtime=5 --iodepth=128 --bs_unaligned=1 --norandommap=1  --verify=0  --serialize_overlap=1 --numjobs=1 --ramp_time=10 --name=test" 
    # 4k   rw / rr
    fio ${fio_common_parameter} --filename="${filename}" --bs=128k --readwrite=write 
    fio ${fio_common_parameter} --filename="${filename}" --bs=128k --readwrite=read  
    # 128k sw / sr
    fio ${fio_common_parameter} --filename="${filename}" --bs=4k --readwrite=randwrite 
    fio ${fio_common_parameter} --filename="${filename}" --bs=4k --readwrite=randread  
}

function f_check_overhead
{
    iops_w=(0 0 0 0) # sr sw rr rw
    iops_wo=(0 0 0 0)
    bw_w=(0 0 0 0)
    bw_wo=(0 0 0 0)
    bw_unit=(0 0 0 0)
    lat_avg_w=(0 0 0 0)
    lat_avg_wo=(0 0 0 0)
    lat_avg_unit=(0 0 0 0)
    # lat_tail_w=(0 0 0 0)
    # lat_tail_wo=(0 0 0 0)
    # lat_tail_unit=(0 0 0 0)

    f_cal "w_air"
    f_cal "wo_air"

    blue_echo "#1 Check 128k Seq.Write"; echo " "
    f_print_result 0; echo " "
    blue_echo "#2 Check 128k Seq.Read"; echo " "
    f_print_result 1; echo " "
    blue_echo "#3 Check 4k Rand.Write"; echo " "
    f_print_result 2; echo " "
    blue_echo "#4 Check 4k Rand.Read"; echo " "
    f_print_result 3; echo " "
}

function f_print_result
{
    TESTS=$((${TESTS}+3)) # iops, bw, lat.avg
    # IOPS
    result=$(echo "scale=2; (${iops_w[$1]}-${iops_wo[$1]})*100/${iops_wo[$1]}" | bc)
    st=`echo "${result} > -6" | bc`
    if [ ${st} -eq 1 ] ; then
        green_echo "[PASS]"
    else
        red_echo "[FAIL]"
        FAILURES=$((${FAILURES}+1))
    fi
    printf "   IOPS           : %15.2f (wo/air)  %15.2f (w/air) %15.2f%% (ratio)\n" ${iops_wo[$1]} ${iops_w[$1]} ${result}
    # BANDWIDTH
    result=$(echo "scale=2; (${bw_w[$1]}-${bw_wo[$1]})*100/${bw_wo[$1]}" | bc)
    st=`echo "${result} > -6" | bc`
    if [ ${st} -eq 1 ] ; then
        green_echo "[PASS]"
    else
        red_echo "[FAIL]" 
        FAILURES=$((${FAILURES}+1))
    fi
    printf "   BW(MiB/s)      : %15.2f (wo/air)  %15.2f (w/air) %15.2f%% (ratio)\n" ${bw_wo[$1]} ${bw_w[$1]} ${result}
    # LAT AVG
    result=$(echo "scale=2; (${lat_avg_w[$1]}-${lat_avg_wo[$1]})*100/${lat_avg_wo[$1]}" | bc)
    st=`echo "${result} < 6" | bc`
    if [ ${st} -eq 1 ] ; then
        green_echo "[PASS]"
    else
        red_echo "[FAIL]" 
        FAILURES=$((${FAILURES}+1))
    fi
    printf "   Lat.Avg(usec)  : %15.2f (wo/air)  %15.2f (w/air) %15.2f%% (ratio)\n" ${lat_avg_wo[$1]} ${lat_avg_w[$1]} ${result}
    # LAT TAIL
    # result=$(echo "scale=2; (${lat_tail_w[$1]}-${lat_tail_wo[$1]})*100/${lat_tail_wo[$1]}" | bc)
    # st=`echo "${result} < 6" | bc`
    # if [ ${st} -eq 1 ] ; then
    #     green_echo "[PASS]"
    # else
    #     red_echo "[FAIL]" 
    # fi
    # printf "   Lat.Tail(%s) : %15.2f (wo/air)  %15.2f (w/air) %15.2f%% (ratio)\n" ${lat_tail_unit[$1]} ${lat_tail_wo[$1]} ${lat_tail_w[$1]} ${result}
}

function f_cal
{
    if [ "$1" == "w_air" ] ; then
	    local file_name=`echo "${FILE_W_AIR}"`
        local str="w"
    elif [ "$1" == "wo_air" ] ; then
        local file_name=`echo "${FILE_WO_AIR}"`
        local str="wo"
    else
        return
    fi
    
    # IOPS 
    local iops=`cat ${file_name} | grep iops | cut -d "," -f3 | cut -d "=" -f2 | sed 's/[^0-9.]//g'`
    local iops_arr=( ${iops} )   
    for seq in {0..3} # sr sw rr rw
    do
        for i in {1..4} # calc 5 times avg
        do
            iops_arr[${seq}]=`echo "scale=2; ${iops_arr[${seq}]}+${iops_arr[${i}*4+${seq}]}" | bc`
        done 
        eval iops_${str}[${seq}]=`echo "scale=2; ${iops_arr[${seq}]}/5" | bc`
    done
  
    # BW
    local bw=`cat ${file_name} | grep bw | grep avg | cut -d "," -f4 | cut -d "=" -f2 | sed 's/[^0-9.]//g'`
    local bw_u=`cat ${file_name} | grep bw | grep avg | cut -d ")" -f1 | cut -d "(" -f2`
    local bw_arr=( ${bw} )
    bw_unit=( ${bw_u} )
    for i in {0..19} # unit -> MiB/s
    do
        if [ "${bw_unit[${i}]}" == "KiB/s" ] ; then
            bw_arr[${i}]=`echo "scale=2; ${bw_arr[${i}]}/1000" | bc`
        fi
    done
    for seq in {0..3} # sr sw rr rw
    do
        for i in {1..4} # calc 5 times avg
        do
            bw_arr[${seq}]=`echo "scale=2; ${bw_arr[${seq}]}+${bw_arr[${i}*4+${seq}]}" | bc`
        done 
        eval bw_${str}[${seq}]=`echo "scale=2; ${bw_arr[${seq}]}/5" | bc`
    done

    # LAT avg
    local lat_avg=`cat ${file_name} | grep lat | grep -v clat | grep -v slat| grep avg | cut -d "," -f3 | cut -d "=" -f2 | sed 's/[^0-9.]//g'`
    local lat_avg_u=`cat ${file_name} | grep lat | grep -v clat | grep -v slat | grep avg | cut -d ")" -f1 | cut -d "(" -f2`
    local lat_avg_arr=( $lat_avg )
    lat_avg_unit=( ${lat_avg_u} )
    for i in {0..19} # unit -> usec
    do
        if [ "${lat_avg_unit[${i}]}" == "nsec" ] ; then
            lat_avg_arr[${i}]=`echo "scale=2; ${lat_avg_arr[${i}]}/1000" | bc`
        fi
    done
    for seq in {0..3} # sr sw rr rw
    do
        for i in {1..4} # calc 5 times avg
        do
            lat_avg_arr[${seq}]=`echo "scale=2; ${lat_avg_arr[${seq}]}+${lat_avg_arr[${i}*4+${seq}]}" | bc`
        done 
        eval lat_avg_${str}[${seq}]=`echo "scale=2; ${lat_avg_arr[${seq}]}/5" | bc`
    done

    # LAT tail 99.90
    # local lat_tail=`cat ${file_name} | grep 99.00th | cut -d "=" -f2 | cut -d "]" -f1 | cut -d "[" -f2 | sed 's/[^0-9.]//g'`
    # local lat_tail_u=`cat ${file_name} | grep percentiles | cut -d ")" -f1 | cut -d "(" -f2`
    # local lat_tail_arr=( $lat_tail )
    # lat_tail_unit=( ${lat_tail_u} )
    # for seq in {0..3} # sr sw rr rw
    # do
    #     for i in {1..4} # calc 5 times avg
    #     do
    #         lat_tail_arr[${seq}]=`echo "scale=2; ${lat_tail_arr[${seq}]}+${lat_tail_arr[${i}*4+${seq}]}" | bc`
    #     done 
    #     eval lat_tail_${str}[${seq}]=`echo "scale=2; ${lat_tail_arr[${seq}]}/5" | bc`
    # done
}

function f_create_xml
{
    local timestamp=`date +%Y-%m-%d%a%H:%M:%S`
    local log_type 
    local workload
    local result_feature

    exec 3<> ${FILE_XML}
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >&3
    echo "<testsuites tests=\"${TESTS}\" failures=\"${FAILURES}\" disables=\"0\" error=\"0\" timestamp=\"${timestamp}\" time=\"20.000\" name=\"overhead_test_result.xml\">" >&3
    echo " <testsuite name=\"overhead_test_result\" tests=\"${TESTS}\" failures=\"${FAILURES}\" disables=\"0\" error=\"0\" time=\"20.000\">" >&3
    while read l; do
        if [[ "$l" =~ "PERF" ]] ; then
            log_type="Log.Perf"
        elif [[ "$l" =~ "LAT" ]] ; then
            log_type="Log.Lat" 
        fi

        if [[ "$l" =~ "Seq.Write" ]] ; then
            workload="128k_seq_write"
        elif [[ "$l" =~ "Seq.Read" ]] ; then
            workload="128k_seq_read"
        elif [[ "$l" =~ "Rand.Write" ]] ; then
            workload="4k_rand_write"
        elif [[ "$l" =~ "Rand.Read" ]] ; then
            workload="4k_rand_read"
        fi

        if [[ "$l" =~ "IOPS" ]] ; then
            result_feature="iops"
        elif [[ "$l" =~ "BW" ]] ; then
            result_feature="bandwidth"
        elif [[ "$l" =~ "Lat.Avg" ]] ; then
            result_feature="latency.avg"
        fi

        if [[ "$l" =~ "PASS" ]] ; then
            echo "  <testcase name=\"${log_type}-${workload}-${result_feature}\" status=\"run\" result=\"completed\" time=\"20.000\" classname=\"overhead_test_result\" />" >&3
        elif [[ "$l" =~ "FAIL" ]] ; then
            local str=`echo "$l" | cut -d ":" -f2`
            echo "  <testcase name=\"${log_type}-${workload}-${result_feature}\" status=\"run\" result=\"completed\" time=\"20.000\" classname=\"overhead_test_result\">" >&3
            echo "  <failure message=\"$str\" type=\"\"><![CDATA[]]></failure>" >&3
            echo "  </testcase>" >&3
        fi
    done < ${FILE_RESULT}
    echo " </testsuite>" >&3
    echo "</testsuites>" >&3
    exec 3>&-
}

# main
if [ "$#" -lt 1 ] ; then
	f_usage
fi
TARGET_IP_ADDR=$1

rm -rf ${FILE_RESULT}
rm -rf ${FILE_W_AIR_PERF}
rm -rf ${FILE_W_AIR_LAT}
rm -rf ${FILE_WO_AIR}
rm -rf ${FILE_W_AIR}
rm -rf ${FILE_XML}
# without air
f_evaluate "wo_air" 
# with air - perf
f_evaluate "w_air_perf" 
echo "----------------------------------" >> ${FILE_RESULT}
echo "- Overhead Test: 1. AIR LOG PERF -" >> ${FILE_RESULT}
echo "----------------------------------" >> ${FILE_RESULT}
f_check_overhead >> ${FILE_RESULT}
mv ${FILE_W_AIR} ${FILE_W_AIR_PERF}
# with air - lat
f_evaluate "w_air_lat" 
echo " " >> ${FILE_RESULT}
echo "----------------------------------" >> ${FILE_RESULT}
echo "- Overhead Test: 2. AIR LOG LAT  -" >> ${FILE_RESULT}
echo "----------------------------------" >> ${FILE_RESULT}
f_check_overhead >> ${FILE_RESULT}
mv ${FILE_W_AIR} ${FILE_W_AIR_LAT}
cat ${FILE_RESULT}

f_create_xml
