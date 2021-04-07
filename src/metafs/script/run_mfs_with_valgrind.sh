#!/bin/bash

# temporary change working directory where resides the script file
if [ -f $0 ]; then
cd $(dirname $0)
fi

if ! type "valgrind" > /dev/null; then
	echo "valgrind tool is not found. Please install valgrind"
	exit 1
fi

if [ ! -e ../mfs ]; then
	echo "'mfs' binary is not available. Please build mfs first"
	exit 1
fi

#sudo valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all --track-origins=yes ../mfs --gtest_filter=*MetaVolumeOpen
#sudo valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all --track-origins=yes ../mfs --gtest_filter=*ProcessNewReqFailDuetoAttemptingCloseForInvalidFile
ret=0
valgrind_log="valgrind.log"
rm -rf ${valgrind_log}

> ${valgrind_log}
sudo valgrind --tool=memcheck --leak-check=yes --track-origins=yes ../mfs 2> ${valgrind_log}
# sudo valgrind ../mfs 2> ${valgrind_log}
grep -q "no leaks" ${valgrind_log}
if [[ $? -ne 0 ]]; then
	echo "Test failed...Stop. See log file..${valgrind_log}"
	trap ERR

	# when the script called by another script,
	if [ "$0" != "$BASH_SOURCE" ]; then
		exit 1;
	fi
else 
	rm -rf ${valgrind_log}
fi

exit 0