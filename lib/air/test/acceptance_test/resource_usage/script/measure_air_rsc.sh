#!/bin/bash

interval=1
num_loop=5
num_samples=50
padding=5
num_air_thread=0

#echo "========== mock_base w/ AIR 0 node =========="
empty_cpu_min=(100.0 100.0 100.0 100.0 100.0)
empty_cpu_max=(0.0 0.0 0.0 0.0 0.0)
empty_air_cpu=(0.0 0.0 0.0 0.0 0.0)
empty_air_mem=0
empty_mem_max=0
empty_mem_min=999999999
loop=$(seq 1 $num_loop)
for l in $loop
do
#echo "loop #" $l

# Run binary
taskset -c 9 ../../../../bin/rsc_empty_air &
# Get PID
temp=$(ps -ef | grep empty_air | head -n 1)
ps_result=($temp)
pid=${ps_result[1]}
#echo $pid

# Limit CPU usage
#taskset -c 9 cpulimit -e empty_air -l 250 &

sleep $padding

for j in $(seq 1 $num_samples)
do

# Get CPU usage of each thread
temp=$(ps -p $pid -L -o pcpu)
#echo "#$j " $temp
cpu_usage=($temp)
index=0
arr_index=0
for i in "${cpu_usage[@]}"
do
    if [ $index -gt 1 ] && [ $index -ne 3 ] && [ $index -ne 4 ]
    then
        if [ $arr_index -eq 0 ]; then
            if [[ ($i == 0.0) ]]; then
                arr_index=`expr $arr_index + 1`
                index=`expr $index + 1`
                continue
            else
#                echo "not zero " $i
                empty_air_cpu[$arr_index]=`bc -l <<< "${empty_air_cpu[$arr_index]} + $i"`
                num_air_thread=`expr $num_air_thread + 1`
                arr_index=`expr $arr_index + 1`
            fi
        else
            empty_air_cpu[$arr_index]=`bc -l <<< "${empty_air_cpu[$arr_index]} + $i"`

            if [ $(bc <<< "${empty_cpu_max[$arr_index]} < $i") -eq 1 ]; then
                empty_cpu_max[$arr_index]=$i
            fi
            if [ $(bc <<< "${empty_cpu_min[$arr_index]} > $i") -eq 1 ]; then
                empty_cpu_min[$arr_index]=$i
            fi
            arr_index=`expr $arr_index + 1`
        fi
    fi
    index=`expr $index + 1`
done
#printf ' cpu usage: %s\n' "${empty_air_cpu[@]}"
#printf ' cpu max: %s\n' "${empty_cpu_max[@]}"
#printf ' cpu min: %s\n' "${empty_cpu_min[@]}"

# Get memory usage
temp=$(pmap -x $pid | tail -n 1)
pmap_result=($temp)
empty_air_mem=`expr $empty_air_mem + ${pmap_result[2]}`
if [[ ($empty_mem_max < ${pmap_result[2]}) ]]; then
    empty_mem_max=${pmap_result[2]}
fi
if [[ ($empty_mem_min > ${pmap_result[2]}) ]]; then
    empty_mem_min=${pmap_result[2]}
fi

sleep $interval
done    # End measure empty_air
#echo " mem usage: " $empty_air_mem
#echo " mem max: " $empty_mem_max
#echo " mem min: " $empty_mem_min

pkill -9 rsc_empty_air
#pkill -9 cpulimit

done    # End loop

#echo " sum value: " ${empty_air_cpu[*]}
# Calculate average CPU and memory usage
for i in 0 1 2 3 4
do
    if [ $i -eq 0 ]; then
        cpu_usage=${empty_air_cpu[$i]}
#        echo "  AIR thread, usage: " $cpu_usage ", non-zero count: " $num_air_thread
        if [ $num_air_thread -ne 0 ]; then
            cpu_usage=`bc -l <<< "$cpu_usage / $num_air_thread"`
        fi
    else
        cpu_usage=`bc -l <<< "${empty_air_cpu[$i]} / ($num_samples * $num_loop)"`
    fi
    empty_air_cpu[$i]=$cpu_usage
done
mem_usage=`bc -l <<< "$empty_air_mem / ($num_samples * $num_loop)"`
empty_air_mem=$mem_usage

#echo " empty air cpu usage: " ${empty_air_cpu[*]}
#echo " empty air mem usage: " $empty_air_mem


#echo
#echo "========== mock_base w/o AIR =========="
no_air_cpu=(0.0 0.0 0.0 0.0 0.0)
no_cpu_max=(0.0 0.0 0.0 0.0 0.0)
no_cpu_min=(0.0 100.0 100.0 100.0 100.0)
no_air_mem=0
no_mem_max=0
no_mem_min=999999999
for l in $loop
do
#echo "loop #" $l

# Run binary
taskset -c 9 ../../../../bin/rsc_no_air &
# Get PID
temp=$(ps -ef | grep no_air | head -n 1)
ps_result=($temp)
pid=${ps_result[1]}

# Limit CPU usage
#taskset -c 9 cpulimit -e no_air -l 250 &

sleep $padding

for j in $(seq 1 $num_samples)
do

# Get CPU usage of each thread
temp=$(ps -p $pid -L -o pcpu)
#echo "#$j " $temp
cpu_usage=($temp)
index=0
arr_index=1
for i in "${cpu_usage[@]}"
do
    if [ $index -gt 2 ]; then
        no_air_cpu[$arr_index]=`bc -l <<< "${no_air_cpu[$arr_index]} + $i"`

        if [ $(bc <<< "${no_cpu_max[$arr_index]} < $i") -eq 1 ]; then
            no_cpu_max[$arr_index]=$i
        fi
        if [ $(bc <<< "${empty_cpu_min[$arr_index]} > $i") -eq 1 ]; then
            no_cpu_min[$arr_index]=$i
        fi
        arr_index=`expr $arr_index + 1`
    fi
    index=`expr $index + 1`
done
#printf ' cpu usage: %s\n' "${no_air_cpu[@]}"
#printf ' cpu max: %s\n' "${no_cpu_max[@]}"
#printf ' cpu min: %s\n' "${no_cpu_min[@]}"

# Get memory usage
temp=$(pmap -x $pid | tail -n 1)
pmap_result=($temp)
no_air_mem=`expr $no_air_mem + ${pmap_result[2]}`
if [[ ($no_mem_max < ${pmap_result[2]}) ]]
then
    no_mem_max=${pmap_result[2]}
fi
if [[ ($no_mem_min > ${pmap_result[2]}) ]]
then
    no_mem_min=${pmap_result[2]}
fi

sleep $interval
done    # End mesaure no_air

#echo " mem usage: " $no_air_mem
#echo " mem max: " $no_mem_max
#echo " mem min: " $no_mem_min

pkill -9 rsc_no_air
#pkill -9 cpulimit

done    # End loop
# Calculate average CPU and memory usage
for i in 1 2 3 4
do
    cpu_usage=`bc -l <<< "${no_air_cpu[$i]} / ($num_samples * $num_loop)"`
    no_air_cpu[$i]=$cpu_usage
done
mem_usage=`bc -l <<< "$no_air_mem / ($num_samples * $num_loop)"`
no_air_mem=$mem_usage

#echo " no air cpu usage: " ${no_air_cpu[*]}
#echo " no air mem usage: " $no_air_mem


#echo
#echo "========== mock_base w/ AIR 10 perf nodes =========="
perf_cpu_min=(100.0 100.0 100.0 100.0 100.0)
perf_cpu_max=(0.0 0.0 0.0 0.0 0.0)
perf_air_cpu=(0.0 0.0 0.0 0.0 0.0)
perf_air_mem=0
perf_mem_max=0
perf_mem_min=999999999
num_air_thread=0

for l in $loop
do
#echo "loop #" $l

# Run binary
taskset -c 9 ../../../../bin/rsc_perf_node &
# Get PID
temp=$(ps -ef | grep perf_node | head -n 1)
ps_result=($temp)
pid=${ps_result[1]}

# Limit CPU usage
#cpulimit -e perf_node -l 250 &

sleep $padding

for j in $(seq 1 $num_samples)
do

# Get CPU usage of each thread
temp=$(ps -p $pid -L -o pcpu)
#echo "#$j " $temp
cpu_usage=($temp)
index=0
arr_index=0
for i in "${cpu_usage[@]}"
do
    if [ $index -gt 1 ] && [ $index -ne 3 ] && [ $index -ne 4 ]; then
        if [ $arr_index -eq 0 ]; then
            if [[ ($i == 0.0) ]]; then
                arr_index=`expr $arr_index + 1`
                index=`expr $index + 1`
                continue
            else
                perf_air_cpu[$arr_index]=`bc -l <<< "${perf_air_cpu[$arr_index]} + $i"`
                num_air_thread=`expr $num_air_thread + 1`
                arr_index=`expr $arr_index + 1`

            fi
        else
            perf_air_cpu[$arr_index]=`bc -l <<< "${perf_air_cpu[$arr_index]} + $i"`
            if [ $(bc <<< "${perf_cpu_max[$arr_index]} < $i") -eq 1 ]; then
                perf_cpu_max[$arr_index]=$i
            fi
            if [ $(bc <<< "${perf_cpu_min[$arr_index]} > $i") -eq 1 ]; then
               perf_cpu_min[$arr_index]=$i
           fi
           arr_index=`expr $arr_index + 1`
        fi
    fi
    index=`expr $index + 1`
done
#printf ' cpu usage: %s\n' "${perf_air_cpu[@]}"
#printf ' cpu max: %s\n' "${perf_cpu_max[@]}"
#printf ' cpu min: %s\n' "${perf_cpu_min[@]}"

# Get memory usage
temp=$(pmap -x $pid | tail -n 1)
pmap_result=($temp)
perf_air_mem=`expr $perf_air_mem + ${pmap_result[2]}`
if [[ ($perf_mem_max < ${pmap_result[2]}) ]]; then
    perf_mem_max=${pmap_result[2]}
fi
if [[ ($perf_mem_min > ${pmap_result[2]}) ]]; then
    perf_mem_min=${pmap_result[2]}
fi

sleep $interval
done    # End measure perf_air
#echo " mem usage: " $perf_air_mem
#echo " mem max: " $perf_mem_max
#echo " mem min: " $perf_mem_min

pkill -9 rsc_perf_node
#pkill -9 cpulimit

done    # End loop

# Calculate average CPU and memory usage
for i in 0 1 2 3 4
do
    if [ $i -eq 0 ]; then
        cpu_usage=${perf_air_cpu[$i]}
        if [ $num_air_thread -ne 0 ]; then
            cpu_usage=`bc -l <<< "$cpu_usage / $num_air_thread"`
        fi
    else
        cpu_usage=`bc -l <<< "${perf_air_cpu[$i]} / ($num_samples * $num_loop)"`
    fi
    perf_air_cpu[$i]=$cpu_usage
done
mem_usage=`bc -l <<< "$perf_air_mem / ($num_samples * $num_loop)"`
perf_air_mem=$mem_usage

#echo " perf node cpu usage: " ${perf_air_cpu[*]}
#echo " perf node mem usage: " $perf_air_mem


#echo
#echo "========== mock_base w/ AIR 10 multi-latency nodes =========="
lat_cpu_min=(100.0 100.0 100.0 100.0 100.0)
lat_cpu_max=(0.0 0.0 0.0 0.0 0.0)
lat_air_cpu=(0.0 0.0 0.0 0.0 0.0)
lat_air_mem=0
lat_mem_max=0
lat_mem_min=999999999
num_air_thread=0

for l in $loop
do
#echo "loop #" $l

# Run binary
taskset -c 9 ../../../../bin/rsc_lat_node &
# Get PID
temp=$(ps -ef | grep lat_node | head -n 1)
ps_result=($temp)
pid=${ps_result[1]}

# Limit CPU usage
#taskset -c 9 cpulimit -e lat_node -l 250 &

sleep $padding

for j in $(seq 1 $num_samples)
do

# Get CPU usage of each thread
temp=$(ps -p $pid -L -o pcpu)
#echo "#$j " $temp
cpu_usage=($temp)
index=0
arr_index=0
for i in "${cpu_usage[@]}"
do
    if [ $index -gt 1 ] && [ $index -ne 3 ] && [ $index -ne 4 ]
    then
        if [ $arr_index -eq 0 ]; then
            if [[ ($i == 0.0) ]]; then
                arr_index=`expr $arr_index + 1`
                index=`expr $index + 1`
                continue
            else
                lat_air_cpu[$arr_index]=`bc -l <<< "${lat_air_cpu[$arr_index]} + $i"`
                num_air_thread=`expr $num_air_thread + 1`
                arr_index=`expr $arr_index + 1`
            fi
        else
            lat_air_cpu[$arr_index]=`bc -l <<< "${lat_air_cpu[$arr_index]} + $i"`
    
            if [ $(bc <<< "${lat_cpu_max[$arr_index]} < $i") -eq 1 ]; then
                lat_cpu_max[$arr_index]=$i
            fi
            if [ $(bc <<< "${lat_cpu_min[$arr_index]} > $i") -eq 1 ]; then
                lat_cpu_min[$arr_index]=$i
            fi
            arr_index=`expr $arr_index + 1`
        fi
    fi
    index=`expr $index + 1`
done
#printf ' cpu usage: %s\n' "${lat_air_cpu[@]}"
#printf ' cpu max: %s\n' "${lat_cpu_max[@]}"
#printf ' cpu min: %s\n' "${lat_cpu_min[@]}"

# Get memory usage
temp=$(pmap -x $pid | tail -n 1)
pmap_result=($temp)
lat_air_mem=`expr $lat_air_mem + ${pmap_result[2]}`
if [[ ($lat_mem_max < ${pmap_result[2]}) ]]; then
    lat_mem_max=${pmap_result[2]}
fi
if [[ ($lat_mem_min > ${pmap_result[2]}) ]]; then
    lat_mem_min=${pmap_result[2]}
fi

sleep $interval
done    # End measure lat_node
#echo " mem usage: " $lat_air_mem
#echo " mem max: " $lat_mem_max
#echo " mem min: " $lat_mem_min

pkill -9 rsc_lat_node
#pkill -9 cpulimit

done    # End loop

# Calculate average CPU and memory usage
for i in 0 1 2 3 4
do
    if [ $i -eq 0 ]; then
        cpu_usage=${lat_air_cpu[$i]}
        if [ $num_air_thread -ne 0 ]; then
            cpu_usage=`bc -l <<< "$cpu_usage / $num_air_thread"`
        fi
    else
        cpu_usage=`bc -l <<< "${lat_air_cpu[$i]} / ($num_samples * $num_loop)"`
    fi
    lat_air_cpu[$i]=$cpu_usage
done
mem_usage=`bc -l <<< "$lat_air_mem /($num_samples * $num_loop)"`
lat_air_mem=$mem_usage

#echo " ml node cpu usage: " ${lat_air_cpu[*]}
#echo " ml node mem usage: " $lat_air_mem

base_cpu_usage=()
perf_cpu_usage=()
lat_cpu_usage=()
# Calculate CPU usage
for i in 0 1 2 3 4
do
    usage=`bc -l <<< "${empty_air_cpu[$i]} - ${no_air_cpu[$i]}"`
    base_cpu_usage+=($usage)
    usage=`bc -l <<< "${perf_air_cpu[$i]} - ${empty_air_cpu[$i]}"`
    perf_cpu_usage+=($usage)
    usage=`bc -l <<< "${lat_air_cpu[$i]} - ${empty_air_cpu[$i]}"`
    lat_cpu_usage+=($usage)
done

#Calculate memory usage
base_mem_usage=`bc -l <<< "($empty_air_mem - $no_air_mem) / 1024"`
perf_mem_usage=`bc -l <<< "($perf_air_mem - $empty_air_mem) / 1024"`
lat_mem_usage=`bc -l <<< "($lat_air_mem - $empty_air_mem) / 1024"`


echo
echo "===================   AIR CPU & Memory usage   ==================="
echo " [CPU]"
echo "  AIR base CPU usage"
echo "   (empty node usage - no air usage)"
printf '   AIR thread: %.2f %% (%.2f - %.2f)\n' "${base_cpu_usage[0]}" "${empty_air_cpu[0]}" "${no_air_cpu[0]}"
printf '   working thread 0: %.2f %% (%.2f - %.2f)\n' "${base_cpu_usage[1]}" "${empty_air_cpu[1]}" "${no_air_cpu[1]}"
printf '   working thread 1: %.2f %% (%.2f - %.2f)\n' "${base_cpu_usage[2]}" "${empty_air_cpu[2]}" "${no_air_cpu[2]}"
printf '   working thread 2: %.2f %% (%.2f - %.2f)\n' "${base_cpu_usage[3]}" "${empty_air_cpu[3]}" "${no_air_cpu[3]}"
printf '   working thread 3: %.2f %% (%.2f - %.2f)\n' "${base_cpu_usage[4]}" "${empty_air_cpu[4]}" "${no_air_cpu[4]}"
echo
echo "  10 performance nodes (total 40 points including aid) CPU usage"
echo "   (perf node usage - empty node usage)"
printf '   AIR thread: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[0]}" "${perf_air_cpu[0]}" "${empty_air_cpu[0]}"
printf '   working thread 0: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[1]}" "${perf_air_cpu[1]}" "${empty_air_cpu[1]}"
printf '   working thread 1: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[2]}" "${perf_air_cpu[2]}" "${empty_air_cpu[2]}"
printf '   working thread 3: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[3]}" "${perf_air_cpu[3]}" "${empty_air_cpu[3]}"
printf '   working thread 4: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[4]}" "${perf_air_cpu[4]}" "${empty_air_cpu[4]}"
echo
echo "  10 multi-latency node (total 40 points including aid) CPU usage "
echo "   (ml node usage - empty node usage)"
printf '   AIR thread: %.2f %% (%.2f - %.2f)\n' "${lat_cpu_usage[0]}" "${lat_air_cpu[0]}" "${empty_air_cpu[0]}"
printf '   working thread 0: %.2f %% (%.2f - %.2f)\n' "${lat_cpu_usage[1]}" "${lat_air_cpu[1]}" "${empty_air_cpu[1]}"
printf '   working thread 1: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[2]}" "${lat_air_cpu[2]}" "${empty_air_cpu[2]}"
printf '   working thread 3: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[3]}" "${lat_air_cpu[3]}" "${empty_air_cpu[3]}"
printf '   working thread 4: %.2f %% (%.2f - %.2f)\n' "${perf_cpu_usage[4]}" "${lat_air_cpu[4]}" "${empty_air_cpu[4]}"
echo
echo " [Memory]"
printf '   AIR base memory usage: %.2f KB\n' "$base_mem_usage"
printf '   10 performance node memory usage: %.2f KB\n' "$perf_mem_usage"
printf '   10 multi-latency node memory usage: %.2f KB\n' "$lat_mem_usage"
echo "=================================================================="





# Create XML file
timestamp=`date +%Y-%m-%d%a%H:%M:%S`
filename="../air_resource_usage.xml"
num_test=4
single_runtime=250
total_runtime=1000

exec 3<> $filename
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >&3
echo "<testsuites tests=\"3\" failures=\"0\" disables=\"0\" error=\"0\" timestamp=\"${timestamp}\" time=\"$total_runtime\" name=\"resource_usage_result.xml\">" >&3

echo " <testsuite name=\"air_base_usage\" tests=\"2\" failures=\"0\" disables=\"0\" error=\"0\" time=\"$single_runtime\">" >&3
echo "  <testcase name=\"CPU\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"air_base_usage\" />" >&3
echo "  <testcase name=\"Memory\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"air_base_usage\" />" >&3
echo " </testsuite>" >&3

echo " <testsuite name=\"perf_node_usage\" tests=\"2\" failures=\"0\" disables=\"0\" error=\"0\" time=\"$single_runtime\">" >&3
echo "  <testcase name=\"CPU\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"perf_node_usage\" />" >&3
echo "  <testcase name=\"Memory\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"perf_node_usage\" />" >&3
echo " </testsuite>" >&3

echo " <testsuite name=\"ml_node_usage\" tests=\"2\" failures=\"0\" disables=\"0\" error=\"0\" time=\"$single_runtime\">" >&3
echo "  <testcase name=\"CPU\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"ml_node_usage\" />" >&3
echo "  <testcase name=\"Memory\" status=\"run\" result=\"completed\" time=\"$single_runtime\" classname=\"ml_node_usage\" />" >&3
echo " </testsuite>" >&3

echo "</testsuites>" >&3
exec 3>&-
