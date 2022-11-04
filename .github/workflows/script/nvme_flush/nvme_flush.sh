target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

cd ${pos_working_dir}/test/regression/; sudo ./nvme_flush_ci_test.sh -f ${target_fabric_ip}

if [ $? -eq 0 ];
then
    echo "Flush Test Success"
    exit 0
else
    echo "\033[1;41mFlush Test Failed\033[0m" 1>&2
    exit 1
fi