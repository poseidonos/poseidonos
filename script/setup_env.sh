#!/bin/bash

# change working directory to where script exists
cd $(dirname $0)

#echo "Setting up Hugepages..."
#modprobe brd rd_nr=2 rd_size=4194304 max_part=0;
MEMORY_LEVEL_1_32GB=33554432      #(32*1024*1024)
MEMORY_LEVEL_2_256GB=268435456    #(256*1024*1024)

echo 3 > /proc/sys/vm/drop_caches
cd ../lib/spdk/scripts;
./setup.sh reset

totalmem_kb=$(cat /proc/meminfo | grep MemTotal | awk '{print $2}')


if [ "$totalmem_kb" -lt "$MEMORY_LEVEL_1_32GB" ]; then
    let hugemem_kb=${totalmem_kb}/3
    let hugemem_kb=${hugemem_kb}*2

    echo "Setting maximum # of Huge Page Size is 2/3 of Total Memory Size"
elif [ "$totalmem_kb" -lt "$MEMORY_LEVEL_2_256GB" ]; then
    let hugemem_kb=${totalmem_kb}/2

    echo "Setting maximum # of Huge Page Size is 1/2 of Total Memory Size"
else
    let hugemem_kb=128*1024*1024

    echo "Setting maximum # of Huge Page Size is 128GB"
fi

let hugemem_nr=${hugemem_kb}/2/1024;
sudo HUGE_EVEN_ALLOC=yes NRHUGE=${hugemem_nr} ./setup.sh; 
cd -;

#SETUP_CORE_DUMP
ulimit -c unlimited
systemctl disable apport.service
mkdir -p /etc/pos/core
echo "/etc/pos/core/%E.core" > /proc/sys/kernel/core_pattern


#SETUP_MAX_MAP_COUNT
MAX_MAP_COUNT=65535
CURRENT_MAX_MAP_COUNT=$(cat /proc/sys/vm/max_map_count)
echo "Current maximum # of memory map areas per process is $CURRENT_MAX_MAP_COUNT."
if [ "$CURRENT_MAX_MAP_COUNT" -lt "$MAX_MAP_COUNT" ]; then
    echo "Setting maximum # of memory map areas per process to $MAX_MAP_COUNT."
    sudo sysctl -w vm.max_map_count=${MAX_MAP_COUNT}
fi

#modprobe nvme-fabrics
modprobe nvme-fabrics

echo "Setup env. done!"
