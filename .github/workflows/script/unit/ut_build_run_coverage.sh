target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

echo "My IP..."
ifconfig

echo "My dir..."
pwd

echo "Running cmake..."
cd ${pos_working_dir}/test/; sudo cmake . -DPOS_DISABLE_LOGGING=OFF
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to cmake error."
    exit $retVal
fi

echo "Building test files..."
cd ${pos_working_dir}/test/; sudo make -j 12
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to UT build error."
    exit $retVal
fi

echo "Running UTs and ITs (a.k.a. called as basic_tests)"
cd ${pos_working_dir}/test/; sudo make run_basic_tests

echo "Calculating UT/IT code coverage..."
cd ${pos_working_dir}/test/; sudo make run_cov