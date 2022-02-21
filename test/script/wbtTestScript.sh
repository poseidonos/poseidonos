#!/bin/bash

#Run it from root ibof directory
#eg bash ./script/wbtTestScript.sh
#cd $(dirname $0)
scriptPath=$(pwd)
echo "In Directory " $scriptPath

ROOT_DIR=$(readlink -f $(dirname $0))/../..

#Relative_Path_root="../.."
BIN_DIR=${ROOT_DIR}/bin

TARGET_MACHINE="pm"

VM_IP_RANGE_1="10.1.11."
VM_IP_RANGE_2="10.100.11."

ARRAYNAME=POSArray

#####################################################################
fileSize1=4096
fileSize2=4096
fileName1="testFile1"
fileName2="testFile2"
fileOffset1=0
fileOffset2=0
#Dont put special chars in data Cause its going though Json parser.
#TODO: Find another way to send data to API
fileData1="sdfsfsdfsdfsdfsdfsdfsdfsdfsdfsfsdfsd.....ABCDEFGHIJKLMNOPQRSTUVWXYZ09876543211234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
fileData2="sdslkhspweoncmspwenajhdfvglqelkhsdsfisdkfjasdkfsdjghwgjwdsfsalghsgsoisligpiuoiuysalgytity53493534538r937085q34850802949wfhgjwl19035820r82wjhwejrhwkhfksdfhksdfsdfsadf"

dataLength1=$(expr ${#fileData1} + ${#fileData2})
dataLength2=$(expr ${#fileData1} + ${#fileData2})

#:TODO Need to get fileDesc from MFS util, but for now using this HARDCODED
fileDesc1=3
fileDesc2=4
inputFile="${scriptPath}/../wbtWriteBufferFile"
cliOutput="${scriptPath}/../wbtCliOutput.txt"
InodeOutput="${scriptPath}/../inodeInfo.json"
FilesInfoOutput="${scriptPath}/../filesInfo.json"
InodeInput="${scriptPath}/../inodeInfo.json"

echo "Input Buffer File " $inputFile
echo "Cli Output file " $cliOutput
#####################################################################
#read -p "Want to MountArray?. Enter y or n:" runMountArray
echo "Input in file.\n"
touch $inputFile
echo -n "$fileData1" >> $inputFile
echo -n "$fileData2" >> $inputFile

cwd="/home/ibof/ibofos/"
exec_mode=0
touch $cliOutput

nss="nqn.2019-04.pos:subsystem1"

ip=`hostname -I | awk '{print $NF}'`
volcnt=1

exit_result=0

print_result()
{
    local result=$1
    local expectedResult=$2

    if [ $expectedResult -eq 0 ]; then
        echo -e "\033[1;34m${date} [result] ${result} \033[0m" 1>&2;
    else
        echo -e "\033[1;41m${date} [TC failed] ${result} \033[0m" 1>&2;

        exit_result=1
    fi
}

check_result()
{
    #local result=$1
    #local expectedResult=$2

    cat ${cliOutput} | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)
    
    if [ ${result} -ne 0 ]; then
        print_result "there is a problem" 1
    else
        print_result "CMD is working" 0
    fi
}


check_result_expected_fail()
{
    #local result=$1
    #local expectedResult=$2

    cat ${cliOutput} | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)
    
    if [ ${result} -ne 0 ]; then
        print_result "CMD is working" 0 
    else
        print_result "there is a problem" 1
    fi
}


pause()
{
    echo "Press any key to continue.."
    read -rsn1
}


check_if_it_is_pm()
{
    if [[ "$ip" =~ "$VM_IP_RANGE_1" ]] || [[ "$ip" =~ "$VM_IP_RANGE_2" ]]; then
        return 1
    elif [ "$TARGET_MACHINE" == "pm" ]; then
        return 1
    else
        return 0
    fi
}


while getopts "f:v:" opt
do
    case "$opt" in
        f) 
            ip="$OPTARG"
            ;;
        v) 
            TARGET_MACHINE="vm"
            ;;
    esac
done


echo "------------[Kill & Start poseidonos]----------------------------------"

sudo ${ROOT_DIR}/test/script/kill_poseidonos.sh
sudo ${ROOT_DIR}/script/start_poseidonos.sh
sleep 10

echo ------------[setup poseidonos]-------------------------------------------

${ROOT_DIR}/test/system/io_path/setup_ibofos_nvmf_volume.sh -a ${ip}

echo ------------[setup Done]-------------------------------------------
echo -------------------------------------------------------------------

echo --------------------------------------------------------------------
echo ------------[GC WBT CMDs]-------------------------------------------
echo --------------------------------------------------------------------


IO_PATH_DIR=${ROOT_DIR}/test/system/io_path

sudo ${IO_PATH_DIR}/fio_bench.py --traddr=${ip} --trtype=tcp --readwrite=write \
--io_size=512M --verify=false --bs=128K --time_based=0 \
--run_time=0 --iodepth=4 --file_num=${volcnt} > /dev/null

sudo ${IO_PATH_DIR}/fio_bench.py --traddr=${ip} --trtype=tcp --readwrite=randwrite \
--io_size=256M --verify=false --bs=4K --time_based=0 \
--run_time=0 --iodepth=4 --file_num=${volcnt} > /dev/null

sleep 5

echo -[gc : do_gc ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt do_gc --array $ARRAYNAME --json-res > ${cliOutput}
check_result
echo --------------------------------------------------------------------

echo -[gc : set_gc_threshold ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $ARRAYNAME --normal 10 --urgent 3 --json-res > ${cliOutput}
check_result

echo -[gc : get_gc_threshold ]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_threshold --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[gc : get_gc_status ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_status --array $ARRAYNAME --json-res > ${cliOutput}
check_result
echo --------------------------------------------------------------------

echo ---------------------------------------------------------------------
echo ------------[Map WBT CMDs]-------------------------------------------
echo ---------------------------------------------------------------------
volname="vol1"
volsize=21474836480

echo -[Map : get_map_layout]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_map_layout --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_vsamap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap --name $volname --output VSAMap_vol1.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_vsamap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap --name $volname --input VSAMap_vol1.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_vsamap_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap_entry --name $volname --rba 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_vsamap_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap_entry --name $volname --rba 0 --vsid 1 --offset 1 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_stripemap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap --output StripeMap.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_stripemap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap --input StripeMap.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_stripemap_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap_entry --vsid 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_stripemap_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap_entry --vsid 0 --loc 1 --lsid 123 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_whole_reverse_map]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_whole_reverse_map --output ReverseMapWhole.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_whole_reverse_map]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_whole_reverse_map --input ReverseMapWhole.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_reverse_map]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map --vsid 0 --output ReverseMap_vsid0.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_reverse_map]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map --vsid 0 --input ReverseMap_vsid0.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : read_reverse_map_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map_entry --vsid 0 --offset 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : write_reverse_map_entry]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map_entry --vsid 0 --offset 0 --rba 0 --name vol1 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_bitmap_layout]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_bitmap_layout --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_instant_meta_info]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_instant_meta_info --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_wb_lsid_bitmap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_wb_lsid_bitmap --output wbLsidBitmap.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : set_wb_lsid_bitmap]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_wb_lsid_bitmap --input wbLsidBitmap.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_active_stripe_tail]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_active_stripe_tail --output activeStripeTail.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : set_active_stripe_tail]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_active_stripe_tail --input activeStripeTail.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_current_ssd_lsid]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_current_ssd_lsid --output currentSsdLsid.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : set_current_ssd_lsid]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_current_ssd_lsid --input currentSsdLsid.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_segment_info]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_info --output segmentInfo.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : set_segment_info]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_segment_info --input segmentInfo.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[Map : get_segment_valid_count]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_valid_count --output segValidCount.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo --------------------------------------------------------------------
echo -------------[User Data IO WBT CMDs]--------------------------------
echo --------------------------------------------------------------------
MAXCOUNT=3
count=0
lbaIdx=0
lbaCnt=0

while [ "$count" -le $MAXCOUNT ] # ($MAXCOUNT) 개의 랜덤 정수 발생.
do    

    lbaIdx=$RANDOM 
    let "lbaIdx %= 1024*8"
    lbaCnt=$RANDOM 
    let "lbaCnt %= 1024"

    echo -[IO Path : unvme-ns-${count} : wbt write_raw]------------------------------------------

    ${BIN_DIR}/poseidonos-cli wbt write_raw --dev unvme-ns-${count} --lba ${lbaIdx} --count ${lbaCnt} --pattern 0xdeadbeef --json-res > ${cliOutput}
    check_result 

    check_if_it_is_pm
    let is_pm=$?

    if [ $is_pm -eq 1 ]; then
        
        echo -[IO Path : unvme-ns-${count} : wbt write_uncorrectable_lba]------------------------------------------
        echo -[ wbt write_uncorrectable_lba is not supported at VM Test ]------------------------------------------        

    else 

        echo -[IO Path : unvme-ns-${count} : wbt write_uncorrectable_lba]------------------------------------------
        ${BIN_DIR}/poseidonos-cli wbt write_uncorrectable_lba --dev unvme-ns-${count} --lba ${lbaIdx} --json-res > ${cliOutput}
        check_result
    fi

    echo -[IO Path : unvme-ns-${count} : wbt flush]------------------------------------------
    ${BIN_DIR}/poseidonos-cli wbt flush --array $ARRAYNAME --json-res > ${cliOutput} 
    check_result

    echo -[IO Path : unvme-ns-${count} : wbt read_raw]------------------------------------------
    ${BIN_DIR}/poseidonos-cli wbt read_raw --dev unvme-ns-${count} --lba ${lbaIdx} --count ${lbaCnt} --output dump.bin --json-res > ${cliOutput}


    if [ $is_pm -eq 1 ]; then
        check_result
    else 
        check_result_expected_fail
    fi
  
    let "count += 1"  # 카운터 증가.
done

echo --------------------------------------------------------------------
echo ------------[MetaFs WBT CMDs]------------------------------------------
echo --------------------------------------------------------------------

echo -[MetaFs : mfs_create_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_create_file --name $fileName1 --size $fileSize1 --integrity 0 --access 2 --operation 2 --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_open_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_open_file --name ${fileName1} --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
cat ${cliOutput} | jq ".Response.result.data.returnCode" > result.txt
fileDesc1=$(<result.txt)
check_result

echo -[MetaFs : mfs_create_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_create_file --name $fileName2 --size $fileSize2 --integrity 0 --access 2 --operation 2 --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_open_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_open_file  --name $fileName2 --volume 0 --array $ARRAYNAME --json-res >  ${cliOutput}
cat ${cliOutput} | jq ".Response.result.data.returnCode" > result.txt
fileDesc2=$(<result.txt)
check_result

echo -[MetaFs : mfs_write_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_write_file --fd $fileDesc1 --volume 0 --offset $fileOffset1 --count $dataLength1 --input $inputFile --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_read_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_read_file --fd $fileDesc1 --volume 0 --offset $fileOffset1 --count $dataLength1 --output mfs_read_one.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_write_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_write_file --fd $fileDesc2 --volume 0 --offset $fileOffset2 --count $dataLength2 --input $inputFile --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_read_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_read_file --fd $fileDesc2 --volume 0 --offset $fileOffset2 --count $dataLength2 --output mfs_read_two.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_get_file_size]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_get_file_size --fd $fileDesc1 --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
cat ${cliOutput} | jq ".Response.result.data.returnCode" > result.txt
fileSize=$(<result.txt)
check_result

echo -[MetaFs : mfs_get_aligned_file_io_size]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_get_aligned_file_io_size --fd $fileDesc1 --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
cat ${cliOutput} | jq ".Response.result.data.returnCode" > result.txt
AlignedFileIoSize=$(<result.txt)
check_result

echo -------------------------------------------------------
echo fileDesc1 = ${fileDesc1} fileDesc2 = ${fileDesc2}
echo fileSize = ${fileSize}   AlignedFileIOSize = ${AlignedFileIoSize}
echo -------------------------------------------------------

echo -[MetaFs : mfs_dump_files_list]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_dump_files_list --output $FilesInfoOutput --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
echo ------- [opend files] ---------------------------------
sed 's/},{/\n /g' ../filesInfo.json > result.txt
cat ${scriptPath}/result.txt
echo -------------------------------------------------------
check_result

echo -[MetaFs : mfs_dump_inode_info]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_dump_inode_info --name $fileName1 --volume 0 --output $InodeOutput --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_dump_inode_info]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_dump_inode_info --name $fileName2 --volume 0 --output $InodeOutput --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_write_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_write_file --fd $fileDesc1 --volume 0 --offset $fileOffset1 --count $dataLength1 --input $inputFile --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_read_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_read_file --fd $fileDesc1 --volume 0 --offset $fileOffset1 --count $dataLength1 --output mfs_read_one.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_write_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_write_file --fd $fileDesc2 --volume 0 --offset $fileOffset2 --count $dataLength2 --input $inputFile --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_read_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_read_file --fd $fileDesc2 --volume 0 --offset $fileOffset2 --count $dataLength2 --output mfs_read_two.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_close_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_close_file --fd ${fileDesc1} --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_close_file]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt mfs_close_file --fd ${fileDesc2} --volume 0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[MetaFs : mfs_setup_meta_fio_test]------------------------------------------
# create 1GB sized volume
${BIN_DIR}/poseidonos-cli volume create -v MetaFsTestVol0 --size 1073741824 --maxiops 0 --maxbw 0 -a $ARRAYNAME
check_result
# mount the volume
${BIN_DIR}/poseidonos-cli volume mount -v MetaFsTestVol0 -a $ARRAYNAME
check_result
# check interface for mesuring metafs performance
${BIN_DIR}/poseidonos-cli wbt mfs_setup_meta_fio_test --name MetaFsTestVol --size 0
check_result
# unmount the volume
${BIN_DIR}/poseidonos-cli volume unmount -v MetaFsTestVol0 -a $ARRAYNAME --force
check_result

echo ------- [Created files] ------------
echo fileDesc1 = ${fileDesc1} fileDesc2 = ${fileDesc2} have closed

sed 's/},{/\n /g' $FilesInfoOutput > result.txt
cat ${scriptPath}/result.txt

echo ----------------------------------

echo --------------------------------------------------------------------

echo ------------[JournalManager WBT CMDs]-------------------------------
echo --------------------------------------------------------------------

echo -[JournalManager : get_journal_status ]-----------------------------
${BIN_DIR}/poseidonos-cli wbt get_journal_status --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo --------------------------------------------------------------------

echo ------- [WBT list] -------------------------------------------------
echo --------------------------------------------------------------------
${BIN_DIR}/poseidonos-cli wbt list_wbt --json-res > ${cliOutput}
check_result
echo --------------------------------------------------------------------

echo -----------------------------------------------------------------------
echo ------------[Array WBT CMDs]-------------------------------------------
echo -----------------------------------------------------------------------
echo -[array : translate_device_lba ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt translate_device_lba --array $ARRAYNAME --lsid 0 --offset 10 --json-res > ${cliOutput}
check_result

echo -[array : dump_disk_layout ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt dump_disk_layout --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[array : parity_location ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt parity_location --array $ARRAYNAME --dev unvme-ns-1 --lba 1572864 --json-res > ${cliOutput}
check_result

echo -[array : get_partition_size ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_partition_size --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo --------------------------------------------------------------------

${BIN_DIR}/poseidonos-cli wbt flush_gcov

echo "------------[WBT Test End, Close poseidonos]----------------------------------"
${BIN_DIR}/poseidonos-cli array unmount --array-name $ARRAYNAME --force
${BIN_DIR}/poseidonos-cli system stop --force

echo "------------[MetaFs Performance Test Starts]----------------------------------"
sleep 5
${ROOT_DIR}/test/script/run_metafs_fio_test.sh -f ${ip} -w 1
echo "------------[MetaFs Performance Test Done]----------------------------------"

echo -----------------------------------------------------------------------
echo ------------[ WBT negative Test Starts]--------------------------------
echo -----------------------------------------------------------------------
${ROOT_DIR}/test/script/wbtNegativeTest.sh
echo ------------[ WBT negative Test End]----------------------------------

rm -rf result.txt
rm -rf ${inputFile}
rm -rf ${cliOutput}


if [ $exit_result -eq 0 ]; then
    echo -[ Test Success ] -
else
    echo -[ Test Fail ] -
fi

exit $exit_result

