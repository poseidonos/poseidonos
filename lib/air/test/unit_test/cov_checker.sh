#!/bin/bash



# getting loc data

SRC_DIR=$(dirname $(realpath $0))/../../src

cd $SRC_DIR

wc -l `find . -name '*.h'` > tmp_loc.txt
wc -l `find . -name '*.cpp'` >> tmp_loc.txt

mv tmp_loc.txt ../test/unit_test

cd ../test/unit_test



# getting TOTAL_LOC

declare -i TOTAL_LOC

TOTAL_LOC=0

while read NUM STR
do
    if [ "$STR" == "total" ]; then
        TOTAL_LOC=$TOTAL_LOC+$NUM
    fi
done < tmp_loc.txt

echo -e "\n [total loc]: $TOTAL_LOC \n"



# getting COVERED_LOC

declare -i COVERED_LOC
declare -i MODULE_LOC
declare -i ZERO_LOC

COVERED_LOC=0
MODULE_LOC=0
ZERO_LOC=0

UT_DIRS='ls -d */'
for MODULE_DIR in $UT_DIRS
do
    while read NUM STR
    do
        if [[ "$STR" == *"$MODULE_DIR"* ]]; then
            MODULE_LOC=$MODULE_LOC+$NUM
            COVERED_LOC=$COVERED_LOC+$NUM
        fi
    done < tmp_loc.txt

    if [ $MODULE_LOC -ne 0 ]; then
        echo -e " module $MODULE_DIR: $MODULE_LOC"
    fi
    MODULE_LOC=0
done



# getting test_coverage

declare -i test_coverage

test_coverage=($COVERED_LOC*100)/$TOTAL_LOC

echo -e "\n [test coverage]: $test_coverage% ($COVERED_LOC/$TOTAL_LOC) \n"

rm -rf tmp_loc.txt
