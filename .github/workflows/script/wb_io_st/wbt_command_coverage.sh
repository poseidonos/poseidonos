target_ip=127.0.0.1
target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

echo " "
echo "---- WBT Command Test Start ----"
echo " "

cd ${pos_working_dir}; sudo unlink /bin/sh 
cd ${pos_working_dir}; sudo ln -s /bin/bash /bin/sh

ls ${pos_working_dir}/test/script/wbtTestScript.sh
if [ $? -ne 0 ];
then
    echo " ### This branch does not have a unit_test script.  ### " 
else    
    cd ${pos_working_dir}/test/system/io_path/; sudo ./setup_ibofos_nvmf_volume.sh -f ${target_ip} 
    cd ${pos_working_dir}/test/script/; ./wbtTestScript.sh -f ${target_fabric_ip}

    if [ $? -eq 0 ];
    then
        echo "WBT Command Test Success"
        cd ${pos_working_dir}/test/regression/; sudo echo 0 > wbtcommandtest
        exit 0
    else
        echo "\033[1;41mWBT Command Test Failed\033[0m" 1>&2
        cd ${pos_working_dir}/test/regression/; sudo echo 1 > wbtcommandtest
        exit 1
    fi
fi