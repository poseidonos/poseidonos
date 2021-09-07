#!/bin/bash
for i in {0..95}
do
   if [ -d /sys/devices/system/cpu/cpu$i ];then
       MAX_FREQ=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq`
       echo $MAX_FREQ > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
       echo performance > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor
   else
       break
   fi

done

