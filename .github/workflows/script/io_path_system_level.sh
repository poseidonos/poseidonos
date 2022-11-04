target_ip=127.0.0.1
pos_working_dir="$1"

failed=0

texecc()
{
    echo "[target]" $@;
    cd ${pos_working_dir}; $@
}

accumulate_result()
{
    if [ ${@} -ne 0 ];
    then
        failed=1
    fi
}

cd ${pos_working_dir}/test/system/io_path/; sudo ./rw_test.py -f ${target_ip}
accumulate_result $?
cd ${pos_working_dir}/test/system/io_path/; sudo ./nvmf_test.py -f ${target_ip}
accumulate_result $?
cd ${pos_working_dir}/test/system/io_path/; sudo ./nvmf_io_command_test.py -f ${target_ip}
accumulate_result $?
cd ${pos_working_dir}/test/system/io_path/; sudo ./watchdog_test.py -t 300
accumulate_result $?

echo "Total result ${failed}"

exit $failed