#!/bin/bash
# test.EXE를 감시
#while :
#do
ps -ef|grep "poseidonos" |  grep -v grep |awk '{print "ps -p " $2 " -o vsz,rss"}' | sh
#sleep 5
#done

