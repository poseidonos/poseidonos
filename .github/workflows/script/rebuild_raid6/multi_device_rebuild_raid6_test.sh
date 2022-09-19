target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

multi_device_rebuild_result=0
cd ${pos_working_dir}/test/regression/; sudo ./fault_tolerance_ci_test_for_raid6_multi_device.sh -f ${target_fabric_ip} -i 1
if [ $? -ne 0 ];
then
    multi_device_rebuild_result=1
fi

if [ $multi_device_rebuild_result -eq 0 ];
then
    echo "Multi-device Rebuild RAID6 Test Success"
    cd ${pos_working_dir}/test/regression/; sudo echo 0 > multidevicerebuildraid6test
    exit 0
else
    echo "\033[1;41mRebuild RAID6 Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; sudo echo 1 > multidevicerebuildraid6test
    exit 1
fi