#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../../
CUR_DIR=$(readlink -f $(dirname $0))
FILE_AIR_JSON=/tmp/air_result.json
FILE_TRY_LOG=$(readlink -f $(dirname $0))/try_log.txt
FILE_ACTUAL_LOG=$(readlink -f $(dirname $0))/actual_log.txt
FILE_RESULT=$(readlink -f $(dirname $0))/test_result.txt
FILE_XML=$(readlink -f $(dirname $0))/\data_loss_test.xml
NUM_TEST=0
NUM_THREAD=0
TESTS=0
FAILURES=0

red_echo()   { echo -n -e "\e[1;31;40m $1 \e[0m"; }
green_echo() { echo -n -e "\e[1;32;40m $1 \e[0m"; }
blue_echo()  { echo -n -e "\e[1;36;40m $1 \e[0m"; }

function f_usage
{
	red_echo " Usage: sh $0 [num test] [num thread]"; echo " "
	exit	
}

function f_build
{
    apt-get install -y jq
    cd ${ROOT_DIR}
    make st_data_loss_test cfg=st_data_loss_test
}

function f_run
{
    cd ${ROOT_DIR}
    ./bin/data_loss_test ${NUM_THREAD} >> ${FILE_TRY_LOG}
    for i in $( eval echo {1..${NUM_THREAD}} )
    do 
        cat /tmp/air_result.json | jq .group.UNGROUPED.node.PERF_TEST.objs[${i}-1].iops_write >> ${FILE_ACTUAL_LOG}
    done 
    log_cnt=`cat ${FILE_TRY_LOG}`
    log_cnt_arr=( ${log_cnt} )  
    file_line=`wc -l ${FILE_TRY_LOG} | cut -d " " -f1`
    sum_log_cnt=0
    for i in $( eval echo {1..${file_line}} ) 
    do
        sum_log_cnt=`echo "${sum_log_cnt}+${log_cnt_arr[${i}-1]}" | bc`
    done 

    iops=`cat ${FILE_ACTUAL_LOG}`
    iops_arr=( ${iops} )  
    file_line=`wc -l ${FILE_ACTUAL_LOG} | cut -d " " -f1`
    sum_iops=0
    for i in $( eval echo {1..${file_line}} ) 
    do
        sum_iops=`echo "${sum_iops}+${iops_arr[${i}-1]}" | bc`
    done 

    f_print_result "${sum_log_cnt}" "${sum_iops}" >> ${FILE_RESULT}
}

function f_print_result
{
    TESTS=$((${TESTS}+1))
    try_log_cnt=`echo $1`
    act_log_cnt=`echo $2`
    ratio=`echo "scale=2; (${act_log_cnt}-${try_log_cnt})*100/${try_log_cnt}" | bc`
    result=`echo "${ratio} > -1" | bc` # 1% 
    printf "TEST#%d" ${TESTS}
    if [ ${result} -eq 1 ] ; then
        green_echo "[PASS]"
    else
        red_echo "[FAIL]"
        FAILURES=$((${FAILURES}+1))
    fi
    printf "  TryLogCnt:%12d  ActualLogCnt:%12d  Ratio:%4.2f%%\n" ${try_log_cnt} ${act_log_cnt} ${ratio} 
}

function f_create_xml
{
    local timestamp=`date +%Y-%m-%d%a%H:%M:%S`
    local cnt=0

    exec 3<> ${FILE_XML}
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >&3
    echo "<testsuites tests=\"${TESTS}\" failures=\"${FAILURES}\" disables=\"0\" error=\"0\" timestamp=\"${timestamp}\" time=\"20.000\" name=\"data_loss_test.xml\">" >&3
    echo " <testsuite name=\"data_loss_test_result\" tests=\"${TESTS}\" failures=\"${FAILURES}\" disables=\"0\" error=\"0\" time=\"20.000\">" >&3
    while read l; do
        if [[ "$l" =~ "PASS" ]] ; then
            echo "  <testcase name=\"data_loss_test #${cnt}\" status=\"run\" result=\"completed\" time=\"20.000\" classname=\"data_loss_test_result\" />" >&3
        elif [[ "$l" =~ "FAIL" ]] ; then
            echo "  <testcase name=\"data_loss_test #${cnt}\" status=\"run\" result=\"completed\" time=\"20.000\" classname=\"data_loss_test_result\">" >&3
            echo "  <failure message=\"$l\" type=\"\"><![CDATA[]]></failure>" >&3
            echo "  </testcase>" >&3
        fi
        cnt=$((${cnt}+1))
    done < ${FILE_RESULT}
    echo " </testsuite>" >&3
    echo "</testsuites>" >&3
    exec 3>&-
}

# main
rm -rf ${FILE_XML}
rm -rf ${FILE_RESULT}
if [ "$#" -lt 2 ] ; then
	f_usage
fi
NUM_TEST=${1}
NUM_THREAD=${2}

f_build
blue_echo "-----------------------------------"; echo " " 
blue_echo "-  System Test: Data Loss Test    -"; echo " " 
blue_echo "-----------------------------------"; echo " " 
echo "num test: ${NUM_TEST}" 
echo "num thread: ${NUM_THREAD}"
for i in $( eval echo {1..${NUM_TEST}} ) 
do
    rm -rf ${FILE_AIR_JSON}
    rm -rf ${FILE_TRY_LOG}
    rm -rf ${FILE_ACTUAL_LOG}
    f_run
done
cat ${FILE_RESULT}
f_create_xml
