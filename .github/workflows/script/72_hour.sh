target_fabric_ip=127.0.0.1
pos_working_dir="$1" 

cd ${pos_working_dir}/test/system/longterm; sudo ./70hour_test.py ${target_fabric_ip}
result=$?

if [ $result -eq 0 ];
then
    echo "Test Succeed"
    exit 0
else
    echo "Test Failed"
    exit 1
fi