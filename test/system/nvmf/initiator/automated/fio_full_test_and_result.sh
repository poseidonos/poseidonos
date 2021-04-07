#!/bin/bash
#
CUR_DIR=$(readlink -f $(dirname $0))
ROOT_DIR=$CUR_DIR/../../../../..

#Default is 4 times
TEST_NR=4

LOG_DIR=$CUR_DIR'/fio_full_log'
if [ ! -d $LOG_DIR ]; then
    mkdir $LOG_DIR
fi

PERF_TEST_FILE=$ROOT_DIR/test/system/nvmf/initiator/automated/perf_test_for_ci.sh
RESULT_FILE=$LOG_DIR'/full_result_'`git log | head -n 1 | awk '{print substr($2,0,10)}'`

#If use Patch file, Please invalidate comment 
#DIFF_FILE='full_test.diff'
#git checkout -- ../fio_full_bench.py
#patch -p5 < $DIFF_FILE

SET=$(seq 0 `expr $TEST_NR - 1`)
for index in $SET
do
    INDEXED_RESULT_FILE=$RESULT_FILE'_'$index	
    echo '####### During Test Case  '$INDEXED_RESULT_FILE' ######'
    sudo script -c  $PERF_TEST_FILE -f $INDEXED_RESULT_FILE
done

sudo $CUR_DIR/parse_result.py $RESULT_FILE $TEST_NR > $LOG_DIR/full_result
#sudo $CUR_DIR/convert_csv.py -f $LOG_DIR/full_result -r $1

#If use Patch File, Please invalidate comment 
#git checkout -- ../fio_full_bench.py

