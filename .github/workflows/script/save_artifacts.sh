target_ip=$1
root_dir=$2
benchmark_output_dir="${root_dir}/test/system/benchmark/output"
artifact_dir=${root_dir}/SustainedPerformance

echo "Cleaning up sustained write directory if there's one: ${artifact_dir}"
rm -rf ${artifact_dir}

echo "Creating sustained write directory"
mkdir -p ${artifact_dir}
echo ${artifact_dir}

echo "Copying pifb results to CI server"
cp -r ${benchmark_output_dir}/* ${artifact_dir}/

find ${artifact_dir} 2>/dev/null | head -n 1
ls ${artifact_dir}

echo "clean up previous report"
cd ${root_dir}
rm -rf ${benchmark_output_dir}/*