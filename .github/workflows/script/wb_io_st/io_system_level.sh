target_ip=127.0.0.1
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
echo "Start Library Build test"
echo "target ip: ${target_ip}"
echo "target ibof directory: ${pos_working_dir}"
echo "test ibofos revision: ${test_rev}"
echo "----------------------------------------------------------------"
sudo rm -rf /dev/shm/*
echo 3 > /proc/sys/vm/ 
ls ${pos_working_dir}/test/system/io_path/library_build_test.py

if [ $? -ne 0 ];
then
    echo " ### This branch does not have a library build test script.  ### " 
else
    sudo chown -R $USER:$USER /var/log/pos    
    cd ${pos_working_dir}/test/system/io_path/; sudo ./library_build_test.py -f ${target_ip} 2>/var/log/pos/librarybuildtest.error
    accumulate_result $?
fi

sudo rm -rf /dev/shm/*
echo "Library Build Test Result : ${failed}"

exit $failed