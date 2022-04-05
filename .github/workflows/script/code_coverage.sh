
target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

gcov_dir_on_ci_server=${pos_working_dir}/GcovResult
gcov_utonly_dir_on_ci_server=${pos_working_dir}/GcovResult-UT

texecc()
{
    echo "[target]" $@;
    cd ${pos_working_dir}; sudo $@
}


# clean up previously profiled data
texecc rm -f total.info total_filtered.lcov

# get lcov output
texecc lcov --rc lcov_branch_coverage=1 -c -d src/ -d test/ --output-file total.info

# exclude unnecessary folders
cd ${pos_working_dir}; lcov --rc lcov_branch_coverage=1 -r total.info \"/usr/*\" \"test/*\" \"*/generated/*\" \"/home/psd/ibofos/proto/generated/*\" \"*/unit-tests/*\" \"*/integration-tests/*\" \"*/nlohmann/*\" '*/ibofos/lib/*' '*/air/src/*' '*/spdk_wrapper/caller/*' -o total_filtered.lcov

# post-processing the coverage report. This will produce "coverage_filtered_pos.lcov".
cd ${pos_working_dir}; python test/filter_out_wrong_branches.py total_filtered.lcov

# clean up prev report
texecc rm -rf ${pos_working_dir}/build/coverage-html

# generate report
texecc genhtml coverage_filtered_pos.lcov --output-directory build/coverage-html --rc lcov_branch_coverage=1 --demangle-cpp --legend

echo "Cleaning up gcov dir if there's one: ${gcov_dir_on_ci_server}"
rm -rf ${gcov_dir_on_ci_server}
rm -rf ${gcov_utonly_dir_on_ci_server}

echo "Creating gcov directory: ${gcov_dir_on_ci_server}"
mkdir -p ${gcov_dir_on_ci_server}

echo "Creating gcov directory - UT only: ${gcov_utonly_dir_on_ci_server}"
mkdir -p ${gcov_utonly_dir_on_ci_server}

echo "Copying gcov result back to CI server... (all)"
cp -r ${pos_working_dir}/build/coverage-html/* ${gcov_dir_on_ci_server}/

echo "Copying gcov result back to CI server... (UT only)"
cp -r ${pos_working_dir}/test/build/coverage-html/* ${gcov_utonly_dir_on_ci_server}/

echo "Listing the gcov reports...(for user validation)"
find ${gcov_dir_on_ci_server} 2>/dev/null | head -n 1

echo "Listing the gcov reports...(for user validation) - UT only"
find ${gcov_utonly_dir_on_ci_server} 2>/dev/null | head -n 1