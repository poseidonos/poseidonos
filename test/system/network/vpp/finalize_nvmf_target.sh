NVMF_PID=`ps -ef | grep "ibof_nvmf_tgt" | awk '{i=i+1; if(i==1) {print $2}}'`
kill -9 $NVMF_PID

