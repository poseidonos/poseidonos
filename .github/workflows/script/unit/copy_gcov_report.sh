pos_working_dir="$1"
gcov_dir_on_ci_server=${pos_working_dir}/GcovResult

echo "Cleaning up gcov dir if there's one: ${gcov_dir_on_ci_server}"
rm -rf ${gcov_dir_on_ci_server}

echo "Creating gcov directory: ${gcov_dir_on_ci_server}"
mkdir -p ${gcov_dir_on_ci_server}

echo "Copying gcov result back to CI server..."
cp -r ${pos_working_dir}/test/build/coverage-html/* ${gcov_dir_on_ci_server}/

echo "Listing the gcov reports...(for user validation)"
find ${gcov_dir_on_ci_server}