#!/bin/bash

# Variables
COMMIT_ID_BEGIN=$1
COMMIT_ID_END=$2

SCRIPT_DIR=`dirname $0`
POS_ROOT_DIR=$SCRIPT_DIR/..
DIR_TO_SEARCH="src"

GIT_DIFF_RESULT_FILE=gitdiff.txt
SED_OUT_FILE1=sedout1.txt
SED_OUT_FILE2=sedout2.txt


# Commands
cd $POS_ROOT_DIR

# Collect newly added or changed files (exclude deleted files)
git diff --stat $COMMIT_ID_BEGIN...$COMMIT_ID_END $DIR_TO_SEARCH > $GIT_DIFF_RESULT_FILE

# Collect lines that have '+' character
sed -n /+/p $GIT_DIFF_RESULT_FILE > $SED_OUT_FILE1

# Collect lines that have '|' character
sed -n /\|/p $SED_OUT_FILE1 > $SED_OUT_FILE2

# Collect line diff number
awk '{TOTAL_LINEDIFF_NUM+=$3} END{print "Number of lines added or changed: " TOTAL_LINEDIFF_NUM}' $SED_OUT_FILE2

# Clean up
rm -f $GIT_DIFF_RESULT_FILE $SED_OUT_FILE1 $SED_OUT_FILE2
