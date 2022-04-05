initiator_ip=$1
initiator_password=$2
cwd=$3
test_rev=$4

iexecc()
{
    echo "[initiator]" $@;
    sshpass -p ${initiator_password} ssh -tt root@${initiator_ip} "cd ${cwd}; sudo $@"
}

setup_initiator()
{
    iexecc git fetch -p
    iexecc rm -rf *
    iexecc git reset --hard ${test_rev}
    iexecc ./script/pkgdep.sh
    sshpass -p ${initiator_password} ssh -tt root@${initiator_ip} "cd ${cwd}/lib; sudo cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=y"
    sshpass -p ${initiator_password} ssh -tt root@${initiator_ip} "cd ${cwd}; sudo make CACHE=Y -j 16 -C lib spdk"
    iexecc rm -f AP_POS_*
    iexecc ./script/setup_env.sh
}

setup_initiator