target_ip=127.0.0.1
target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
test_rev="$2"
failed=0

accumulate_result()
{
    if [ ${@} -ne 0 ];
    then
        failed=1
    fi
}



echo "----------------------------------------------------------------"
echo "Start IO Unit test"
echo "target ip: ${target_ip}"
echo "target ibof directory: ${pos_working_dir}"
echo "test ibofos revision: ${test_rev}"
echo "----------------------------------------------------------------"
rm -rf /dev/shm/*

ls ${pos_working_dir}/test/system/io_path/library_build_test.py

if [ $? -ne 0 ];
then
    echo " ### This branch does not have a unit_test script.  ### " 
else    
    cd ${pos_working_dir}/test/system/io_path/; sudo ./library_build_test.py -f ${target_ip} 2>/var/log/pos/unittest.error
    accumulate_result $?
fi

rm -rf /dev/shm/*
echo "IO Unit Test Result : ${failed}"

cd ${pos_working_dir}/test/regression/; sudo echo ${failed} > iounittest

exit 0