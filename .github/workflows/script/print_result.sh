pos_working_dir="$1" 

failed=0
exitValue=0
volumetest=`cd ${pos_working_dir}/test/regression/; sudo cat volumetest`
iotest=`cd ${pos_working_dir}/test/regression/; sudo cat iotest`
exittest=`cd ${pos_working_dir}/test/regression/; sudo cat exittest`
nportest=`cd ${pos_working_dir}/test/regression/; sudo cat nportest`
rebuildtest=`cd ${pos_working_dir}/test/regression/; sudo cat rebuildtest`
sportest=`cd ${pos_working_dir}/test/regression/; sudo cat sportest`
wbtcommandtest=`cd ${pos_working_dir}/test/regression/; sudo cat wbtcommandtest`
iounittest=`cd ${pos_working_dir}/test/regression/; sudo cat iounittest`

echo "\033[1;36m******************************* \033[0m" 1>&2
echo "\033[1;36m**********Test Result********** \033[0m" 1>&2
echo "\033[1;36m******************************* \033[0m" 1>&2

if [ ${volumetest} -eq 0 ];
then
    echo "Volume Test Success"
else
    echo "\033[1;41mVolume Test Failed\033[0m" 1>&2
    failed=1
fi

if [ ${iotest} -eq 0 ];
then
    echo "IO Test Success"
else
    echo "\033[1;41mIO Test Failed\033[0m" 1>&2
    failed=1
fi

if [ ${nportest} -eq 0 ];
then
    echo "NPOR Test Success"
else
    echo "\033[1;41mNPOR Test Failed\033[0m" 1>&2
    failed=1
fi

if [ ${rebuildtest} -eq 0 ];
then
    echo "Rebuild Test Success"
else
    echo "\033[1;41mRebuild Test Failed\033[0m" 1>&2
    failed=1
fi

if [ ${sportest} -eq 0 ];
then
    echo "SPOR Test Success"
else
    echo "\033[1;41mSPOR Test Failed\033[0m" 1>&2
    failed=1
fi

if [ ${wbtcommandtest} -eq 0 ];
then
    echo "WBT Command Test Success"
else
    echo "\033[1;41mWBT Command Test Failed\033[0m" 1>&2
    failed=1
fi


if [ ${iounittest} -eq 0 ];
then
   echo "IO Unit Test Success"
else
   echo "\033[1;41mIO Unit Test Failed\033[0m" 1>&2
   failed=1
fi


if [ ${failed} -eq 0 ];
then
    echo "All Test Success"
    exitValue=0
else
    echo "\033[1;41mSome Test Failed\033[0m" 1>&2
    exitValue=1
fi

echo "\033[1;36m******************************* \033[0m" 1>&2
echo "\033[1;36m******************************* \033[0m" 1>&2

# Program Exit
exit ${exitValue}
