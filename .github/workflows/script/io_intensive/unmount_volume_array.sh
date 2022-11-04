target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

result=0
cd ""${pos_working_dir}""/test/system/io_path; sudo grep "'"\-p"'" event_during_io.py

if [ $? -ne 0 ];
then
    echo "That version does not support event during io python script. So, skip the test"
    exit 0
fi

result=0
cd ${pos_working_dir}/test/system/io_path; sudo ./event_during_io.py -f ${target_fabric_ip} -v
if [ $? -ne 0 ];
then
    result=1
fi

if [ $result -eq 0 ];
then
    echo "IO + Unmount Test Success"
    exit 0
else
    echo "\033[1;41mIO Exception Test Failed\033[0m" 1>&2
    exit 1
fi
