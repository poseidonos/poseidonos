target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

cd ${pos_working_dir}/test/regression/; sudo ./long_term_ci_test.sh  -s 50 -c true # add size 50 option

if [ $? -eq 0 ];
then
    echo "GC Test Success"
    exit 0
else
    echo "\033[1;41mGC Test Failed\033[0m" 1>&2
    exit 1
fi