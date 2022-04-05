target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
test_rev="$2"

echo "Cleaning up a result dir if there's one"
rm -rf ${pos_working_dir}/../UnitTestResult/${test_rev}

echo "Creating a result directory: ${pos_working_dir}/../UnitTestResult/${test_rev}"
mkdir -p ${pos_working_dir}/../UnitTestResult/${test_rev}

echo "Copying JUNIT XML report back to CI server..."
cp ${pos_working_dir}/test/build/test_detail*.xml ${pos_working_dir}/../UnitTestResult/${test_rev}

echo "Listing the XMLs..."
find ${pos_working_dir}/../UnitTestResult/${test_rev}

echo "Creating a UnitTestResult at curr dir"
pwd
mkdir -p UnitTestResult
cp ${pos_working_dir}/../UnitTestResult/${test_rev}/* ./UnitTestResult/