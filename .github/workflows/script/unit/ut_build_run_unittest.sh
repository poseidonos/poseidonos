target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

sudo ${pos_working_dir}/script/pkgdep.sh

echo "My IP..."
ifconfig

echo "My dir..."
pwd
echo "Install library"
cd ${pos_working_dir}/lib/; sudo ./build_lib.sh

echo "Running cmake..."
cd ${pos_working_dir}/test/; sudo cmake . 
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to cmake error."
    cd ${pos_working_dir}/test/regression/; echo ${retVal} > utbuildtest
    exit $retVal
fi

echo "Building test files..."
cd ${pos_working_dir}/test/; sudo make -j 12
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to UT build error."
    exit $retVal
fi

echo 3 > /proc/sys/vm/drop_caches

echo "Running UTs and ITs (a.k.a. called as basic_tests)"
cd ${pos_working_dir}/test/; sudo make run_basic_tests
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Test Failed"
    exit $retVal
fi

echo "Calculating code coverage..."
cd ${pos_working_dir}/test/; sudo make run_cov