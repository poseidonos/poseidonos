target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

sudo ${pos_working_dir}/script/pkgdep.sh

echo "My IP..."
ifconfig

echo "My dir..."
pwd
echo "Install library"
cd ${pos_working_dir}/lib/; sudo ./build_lib.sh

echo "Copying event file..."
sudo mkdir -p /etc/pos
sudo cp ${pos_working_dir}/src/event/pos_event.yaml /etc/pos/pos_event.yaml

echo "Running cmake..."
cd ${pos_working_dir}/test/; sudo cmake . 
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

echo 3 > /proc/sys/vm/drop_caches

echo "Running UTs and ITs (a.k.a. called as basic_tests)"
cd ${pos_working_dir}/test/; sudo make run_basic_tests
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to UT run error."
    exit $retVal
fi

echo "Calculating code coverage..."
cd ${pos_working_dir}/test/; sudo make run_cov
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Cannot proceed due to code coverage error."
    exit $retVal
fi
