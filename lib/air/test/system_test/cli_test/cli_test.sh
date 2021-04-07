#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../..
CLI_CMD="$ROOT_DIR/bin/air_cli "
TEST_CASE_FILE=$ROOT_DIR/test/system_test/cli_test/system_test_tc
AIR_MOCK="$ROOT_DIR/bin/mock_base"

typeset -i num_of_test_run=0

trap 'printf "$0: exit code $? on line $LINENO\nFAIL: $this\n"; exit1' ERR

red_echo() { echo -n -e "\e[1;31;40m $1 \e[0m"; }
green_echo() { echo -n -e "\e[1;32;40m $1 \e[0m"; }
blue_echo() { echo -n -e "\e[1;36;40m $1 \e[0m"; }

function air_mock_enable
{
	parm=$1
	if [ "$parm" == "ON" ]; then
		if [ -z "$(ps -a | grep mock_base)" ]; then
			blue_echo "RUN mock_base..."; echo " "
			$AIR_MOCK &
		fi
	fi

	if [ "$parm" == "OFF" ]; then
        blue_echo "STOP mock_base..."; echo " "
    	killall mock_base
	fi

	sleep 1
}

function exec_cli
{
	cli_command=$1
	expected_result=$2
	let num_of_test_run+=1
	
	# exec
	retval=$($CLI_CMD$cli_command)
	echo "$retval" >> temp
	exec 4<> temp
	actual_result=""
	while read line; do
		cmd=`echo $line | cut -d' ' -f1`
		retval=`echo "$cmd" | grep -c "SUCCESS"`
		if [ $retval -eq 1 ]; then
    		actual_result="$actual_result $cmd"
		fi
		retval=`echo "$cmd" | grep -c "ERROR"`
		if [ $retval -eq 1 ]; then
    		actual_result="$actual_result $cmd"
		fi
	done < temp
	exec 4>&-
	rm -rf temp
	
	# pass
	actual_result=`echo $actual_result`
	if [ "$actual_result" == "$expected_result" ]; then
		green_echo "[PASS]"
		echo " test #$num_of_test_run: $cli_command ";
		return
	fi
	
	# fail
	red_echo "[FAIL]"
	echo " test #$num_of_test_run: $cli_command ";
	red_echo "      Expected result: $expected_result"; echo " "
	red_echo "      Actual result:   $actual_result"; echo " "
	air_mock_enable "OFF"
	exit 1
}
function test_cli
{
	let num_of_test_run=0
	test_name=$1
	interval=$2
	
	echo " "; echo " ";
	blue_echo "*******************************************"; echo " "
	blue_echo "      TEST: $1"; echo " "
	blue_echo "*******************************************"; echo " "
	
	# enable AIR MOCK
	air_mock_enable "ON"

	# enable AIR
	exec_cli "--run=true" "[SUCCESS]"
	
	# exec cli
	exec 3<> $TEST_CASE_FILE
	while read line; do
		if [ "$line" == "$test_name" ]; then
			while IFS=';' read -r com1 com2; do
				if [ "$com1" == "[END]" ]; then
					exec 3>&-
					# disable AIR
					exec_cli "--run=false" "[SUCCESS]"
					# disable AIR MOCK
					air_mock_enable "OFF"
					return
				fi
				com2=`echo $com2`
				exec_cli "$com1" "$com2"
				sleep $interval
			done
		fi
	done < $TEST_CASE_FILE
}

CMD_LIST="enable-node init-node stream-interval set-sampling-ratio"
function check_cmd_dup
{
	cmd1=$1
	cmd2=$2
	num_of_cmd=$3
	is_dup=0
	
	# if duplicated cmds exist, change the result as [ERROR:-7]
	IFS=" " cmd1_arr=(`echo $cmd1`)
	IFS=" " cmd2_arr=(`echo $cmd2`)
	for cmd_list in ${CMD_LIST[@]} ; do
		retval=`echo "$cmd1" | grep -o "$cmd_list" | wc -l`
		if [ $retval -gt 1 ]; then
			is_dup=1	
			i=0
			while [ $i -lt $num_of_cmd ]
			do
				retval=`echo "${cmd1_arr[$i]}" | grep -o "$cmd_list" | wc -l`	
				if [ $retval -eq 1 ]; then
					cmd2_arr[$i]="[ERROR:-7]"
				fi
				i=$(($i+1))
			done
		fi
	done

	# call by reference
	if [ $is_dup -eq 1 ]; then
		cmd2=${cmd2_arr[0]}
		i=1
		while [ $i -lt $num_of_cmd ]
		do
		    cmd2="$cmd2\ ${cmd2_arr[$i]}"
			i=$(($i+1))
		done
		eval ${4}=$cmd2
	fi
}

function test_multi_cmd
{
	let num_of_test_run=0
	num_of_cmd=$1
	num_of_exec=$2
	interval=$3
	num_of_lines=`cat $TEST_CASE_FILE | wc -l`
	
	echo " "; echo " ";
	blue_echo "*******************************************"; echo " "
	blue_echo "      TEST: MULTI CMD (num of cmd: $num_of_cmd)"; echo " "
	blue_echo "*******************************************"; echo " "
	
	#enable AIR MOCK
	air_mock_enable "ON"

	#enable AIR
	exec_cli "--run=true" "[SUCCESS]"

	#exec multi cli
	cnt=0
	while [ $cnt -lt $num_of_exec ]
	do
		j=0
		multi_cmd1=""
		multi_cmd2=""
		while [ $j -lt $num_of_cmd ]
		do
			rand_line_num=`shuf -i1-$num_of_lines -n1`
			content=`cat $TEST_CASE_FILE | sed -n ${rand_line_num}p`
			cmd1=`echo $content | cut -d';' -f1`
			cmd2=`echo $content | cut -d';' -f2`

			if [ `echo $cmd1 | grep -` ]; then
				#cmd2=`echo $cmd2`
    			multi_cmd1="$multi_cmd1 $cmd1"
    			multi_cmd2="$multi_cmd2 $cmd2"
				multi_cmd2=`echo $multi_cmd2`
				j=$(($j+1))
			fi
		done
	
		check_cmd_dup "$multi_cmd1" "$multi_cmd2" $num_of_cmd "multi_cmd2"
		exec_cli "$multi_cmd1" "$multi_cmd2"
		sleep $interval
		cnt=$(($cnt+1))
	done
	
	#disable AIR
	exec_cli "--run=false" "[SUCCESS]"

	#disable AIR MOCK
	air_mock_enable "OFF"
}

# main
# test_cli "test name" interval(s)
# test_multi_cmd num_of_cmd num_of_exec interval(s)

# System Test (Equivalence Test / Boundary Test / Decision Table Test)
test_cli "[EQUIVALENCE_VALID]"   0.001
test_cli "[EQUIVALENCE_INVALID]" 0.001
test_cli "[BOUNDARY_VALID]" 0.001
test_cli "[BOUNDARY_INVALID]" 0.001
test_multi_cmd 2 50 0.001
test_multi_cmd 3 50 0.001
test_multi_cmd 4 50 0.001

# Use Case(Random) Test
#test_multi_cmd 1 100 0.1
#test_multi_cmd 2 100 0.1
#test_multi_cmd 3 100 0.1
test_multi_cmd 4 100 0.1

# Bulk cmd over 512(no delay)
#test_multi_cmd 1 1000 0
#test_multi_cmd 2 1000 0
#test_multi_cmd 3 1000 0
#test_multi_cmd 4 1000 0
