#!/bin/bash

#Relative_Path_root="../.."

scriptPath=$(pwd)

ROOT_DIR=$(readlink -f $(dirname $0))/../..
BIN_DIR=${ROOT_DIR}/bin

cliOutput="${scriptPath}/../wbtCliOutput.txt"
exec_mode=0
touch $cliOutput
cwd="/home/ibof/ibofos/"

ip=`hostname -I | awk '{print $NF}'`
#####################################################################
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
    cat ${cliOutput} | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)
    
    if [ ${result} -ne 0 ]; then
        print_result "CMD is working" 0 
    else
        print_result "there is a problem" 1
    fi
}

#####################################################################
ARRAYNAME=POSArray
volname="vol1"
volsize=21474836480

INVALID_ARRAYNAME="ABCDE"
invalid_volname="invalidvol"

lbaIdx=$RANDOM 
let "lbaIdx %= 1024*8"
lbaCnt=$RANDOM 
let "lbaCnt %= 1024"


echo ----------------------------------------------------------------------------
echo "------------[Kill & Start poseidonos]----------------------------------"

sudo ${ROOT_DIR}/test/script/kill_poseidonos.sh
sudo ${ROOT_DIR}/script/start_poseidonos.sh
sleep 10

echo ------------[setup poseidonos]-------------------------------------------
${ROOT_DIR}/test/system/io_path/setup_ibofos_nvmf_volume.sh -a ${ip}

echo ------------[setup Done]-------------------------------------------
echo -------------------------------------------------------------------

#####################################################################
echo -------------------------------------------------------------------
echo ------------[Map WBT CMDs with negative cases]---------------------
echo -------------------------------------------------------------------
echo -[Map : get_map_layout_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_map_layout --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_map_layout_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_map_layout --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap --name $volname --output VSAMap_vol1.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap --name $invalid_volname --output VSAMap_vol1.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap --name $volname --output VSAMap_vol1.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap --name $volname --input VSAMap_vol1.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_nagative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap --name $invalid_volname --input VSAMap_vol1.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap --name $volname --input VSAMap_vol1.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap_entry --name $volname --output VSAMap_vol1.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap_entry --name $invalid_volname --output VSAMap_vol1.bin --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_vsamap_entry_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_vsamap_entry --name $volname --output VSAMap_vol1.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap_entry --name $volname --rba 0 --vsid 1 --offset 1 --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap_entry --name $invalid_volname --rba 0 --vsid 1 --offset 1 --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_vsamap_entry_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_vsamap_entry --name $volname --rba 0 --vsid 1 --offset 1 --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_stripemap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap --output StripeMap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_stripemap_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap --output StripeMap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_stripemap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap --input StripeMap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_stripemap_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap --input StripeMap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_stripemap_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap_entry --vsid 0 --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_stripemap_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_stripemap_entry --vsid 0 --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_stripemap_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap_entry --vsid 0 --loc 1 --lsid 123 --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_stripemap_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_stripemap_entry --vsid 0 --loc 1 --lsid 123 --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_whole_reverse_map_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_whole_reverse_map --output ReverseMapWhole.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_whole_reverse_map_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_whole_reverse_map --output ReverseMapWhole.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_whole_reverse_map_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_whole_reverse_map --input ReverseMapWhole.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_whole_reverse_map_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_whole_reverse_map --input ReverseMapWhole.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_reverse_map_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map --vsid 0 --output ReverseMap_vsid0.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_reverse_map_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map --vsid 0 --output ReverseMap_vsid0.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_reverse_map_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map --vsid 0 --input ReverseMap_vsid0.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_reverse_map_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map --vsid 0 --input ReverseMap_vsid0.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_reverse_map_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map_entry --vsid 0 --offset 0 --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : read_reverse_map_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_reverse_map_entry --vsid 0 --offset 0 --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_reverse_map_entry_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map_entry --vsid 0 --offset 0 --rba 0 --name vol1 --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : write_reverse_map_entry_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_reverse_map_entry --vsid 0 --offset 0 --rba 0 --name vol1 --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_bitmap_layout_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_bitmap_layout --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_bitmap_layout_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_bitmap_layout --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_instant_meta_info_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_instant_meta_info --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_instant_meta_info_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_instant_meta_info --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_wb_lsid_bitmap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_wb_lsid_bitmap --output wbLsidBitmap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_wb_lsid_bitmap_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_wb_lsid_bitmap --output wbLsidBitmap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_wb_lsid_bitmap_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_wb_lsid_bitmap --input wbLsidBitmap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_wb_lsid_bitmap_negative-2]---------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_wb_lsid_bitmap --input wbLsidBitmap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_active_stripe_tail_negative-1]-----------------------------------
${BIN_DIR}/poseidonos-cli wbt get_active_stripe_tail --output activeStripeTail.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_active_stripe_tail_negative-2]----------------------------------
${BIN_DIR}/poseidonos-cli wbt get_active_stripe_tail --output activeStripeTail.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_active_stripe_tail_negative-1]----------------------------------
${BIN_DIR}/poseidonos-cli wbt set_active_stripe_tail --input activeStripeTail.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_active_stripe_tail_negative-2]----------------------------------
${BIN_DIR}/poseidonos-cli wbt set_active_stripe_tail --input activeStripeTail.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_current_ssd_lsid_negative-1]------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_current_ssd_lsid --output currentSsdLsid.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_current_ssd_lsid_negative-2]------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_current_ssd_lsid --output currentSsdLsid.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_current_ssd_lsid_negative-1]------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_current_ssd_lsid --input currentSsdLsid.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_current_ssd_lsid_negative-2]------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_current_ssd_lsid --input currentSsdLsid.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_user_segment_bitmap_negative-1]---------------------------------
${BIN_DIR}/poseidonos-cli wbt get_user_segment_bitmap --output segmentBitmap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_user_segment_bitmap_negative-2]---------------------------------
${BIN_DIR}/poseidonos-cli wbt get_user_segment_bitmap --output segmentBitmap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_user_segment_bitmap_negative_1]---------------------------------
${BIN_DIR}/poseidonos-cli wbt set_user_segment_bitmap --input segmentBitmap.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_user_segment_bitmap_negative_2]---------------------------------
${BIN_DIR}/poseidonos-cli wbt set_user_segment_bitmap --input segmentBitmap.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_segment_info_negative_1]----------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_info --output segmentInfo.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_segment_info_negative_2]----------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_info --output segmentInfo.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_segment_info_negative_1]----------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_segment_info --input segmentInfo.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : set_segment_info_negative_2]----------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_segment_info --input segmentInfo.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_segment_valid_count_negative-1]--------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_valid_count --output segValidCount.bin --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[Map : get_segment_valid_count_negative-2]--------------------------------
${BIN_DIR}/poseidonos-cli wbt get_segment_valid_count --output segValidCount.bin --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -------------------------------------------------------------------
echo -------------[User Data IO WBT CMDs with negative cases]-----------
echo -------------------------------------------------------------------
echo -[IO Path : wbt write_uncorrectable_lba_negative-1]--------------------------
${BIN_DIR}/poseidonos-cli wbt write_uncorrectable_lba --dev unvme-ns-9999 --lba ${lbaIdx} --output segValidCount.bin --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt write_uncorrectable_lba_negative-2]--------------------------
${BIN_DIR}/poseidonos-cli wbt write_uncorrectable_lba --dev unvme-ns-0 --lba 0xFFFFFFFF --output segValidCount.bin --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt flush_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt flush --array $INVALID_ARRAYNAME --json-res > ${cliOutput} 
check_result_expected_fail

echo -[IO Path : wbt flush_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt flush --array "" --json-res > ${cliOutput} 
check_result_expected_fail

echo -[IO Path : wbt write_raw_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_raw --dev unvme-ns-9999 --lba ${lbaIdx} --count ${lbaCnt} --pattern 0xdeadbeef --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt write_raw_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_raw --dev unvme-ns-0 --lba ${lbaIdx} --count -1 --pattern 0xdeadbeef --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt write_raw_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt write_raw --dev unvme-ns-0 --lba ${lbaIdx} --count ${lbaCnt} --pattern deadbeef --json-res > ${cliOutput}
check_result_expected_fail

# expect PASS
echo -[IO Path : wbt write_raw -call with input parameter]--------------------------
${BIN_DIR}/poseidonos-cli wbt write_raw --dev unvme-ns-0 --lba ${lbaIdx} --count ${lbaCnt} --input segValidCount.bin --json-res > ${cliOutput}
check_result

echo -[IO Path : wbt read_raw_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_raw --dev unvme-ns-9999 --lba ${lbaIdx} --count ${lbaCnt} --output dump.bin --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt read_raw_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_raw --dev unvme-ns-0 --lba ${lbaIdx} --count -1 --output dump.bin --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt read_raw_negative-3]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt read_raw --dev unvme-ns-0 --lba ${lbaIdx} --count ${lbaCnt} --output "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[IO Path : wbt flush_negative-1]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt flush --array $INVALID_ARRAYNAME --json-res > ${cliOutput} 
check_result_expected_fail

echo -[IO Path : wbt flush_negative-2]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt flush --array "" --json-res > ${cliOutput} 
check_result_expected_fail

echo --------------------------------------------------------------------
echo -------[ GC WBT CMDs with negative cases]---------------------------
echo --------------------------------------------------------------------

echo -[gc : set_gc_threshold_negative-1 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $INVALID_ARRAYNAME --normal 10 --urgent 3 --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : set_gc_threshold_negative-2 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array "" --normal 10 --urgent 3 --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : set_gc_threshold_negative-3 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $ARRAYNAME --normal "" --urgent 3 --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : set_gc_threshold_negative-4 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $ARRAYNAME --normal 9 --urgent "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : set_gc_threshold_negative-5 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $ARRAYNAME --normal 1 --urgent -1 --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : set_gc_threshold_negative-6 ]-------------------------------------------
${BIN_DIR}/poseidonos-cli wbt set_gc_threshold --array $ARRAYNAME --normal 1 --urgent 3 --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : get_gc_threshold_negative-1 ]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_threshold --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : get_gc_threshold_negative-2 ]------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_threshold --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : get_gc_status_negative-1 ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_status --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : get_gc_status_negative-2 ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_gc_status --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : do_gc_negative-1 ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt do_gc --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[gc : do_gc_negative-2 ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt do_gc --array "" --json-res > ${cliOutput}
check_result_expected_fail

# gc already running fail
echo -[gc : do_gc_negative-3 ]---------------------------------------------
${BIN_DIR}/poseidonos-cli wbt do_gc --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -----------------------------------------------------------------------
echo ------------[Array WBT CMDs with negative cases]-----------------------
echo -----------------------------------------------------------------------
echo -[array : translate_device_lba_negative-1 ]---------------------------------------
${BIN_DIR}/poseidonos-cli wbt translate_device_lba --array $INVALID_ARRAYNAME --lsid 0 --offset 10 --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : translate_device_lba_negative-2 ]---------------------------------------
${BIN_DIR}/poseidonos-cli wbt translate_device_lba --array "" --lsid 0 --offset 10 --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : dump_disk_layout_negative-1 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt dump_disk_layout --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : dump_disk_layout_negative-2 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt dump_disk_layout --array "" --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : parity_location_negative-1 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt parity_location --array $INVALID_ARRAYNAME --dev unvme-ns-0 --lba 1572864 --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : parity_location_negative-2 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt parity_location --array "" --dev unvme-ns-0 --lba 1572864 --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : parity_location_negative-3 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt parity_location --array $ARRAYNAME --dev unvme-ns-9999 --lba 1572864 --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : get_partition_size_negative-1 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_partition_size --array $INVALID_ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail

echo -[array : get_partition_size_negative-2 ]-----------------------------------------
${BIN_DIR}/poseidonos-cli wbt get_partition_size --array "" --json-res > ${cliOutput}
check_result_expected_fail


echo --------------------------------------------------------------------
echo ------------[ Additional cases]-------------------------------------
echo --------------------------------------------------------------------

echo -[ common: nvme_cli_formatnvm ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt nvme_cli --op 128 --nsid 1 --lbaf 0 --ms 0 --pi 0 --pil 0 --ses 0 --dev unvme-ns-0 --array $ARRAYNAME --json-res > ${cliOutput}
check_result

echo -[ common: nvme_cli_identify ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt nvme_cli --op 6 --nsid 1 --dev unvme-ns-0 --cns 0 --array $ARRAYNAME --output nvme_idns.bin --json-res > ${cliOutput}
check_result

echo -[ common: nvme_cli_identify ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt nvme_cli --op 6 --nsid 1 --dev unvme-ns-0 --cns 1 --array $ARRAYNAME --output nvme_idctrl.bin --json-res > ${cliOutput}
check_result

echo -[ common: nvme_cli_getlogpage ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt nvme_cli --op 2 --nsid 1 --dev unvme-ns-0 --cns 2 --array $ARRAYNAME --output nvme_getlogpage.bin --json-res > ${cliOutput}
check_result

echo -[ common: nvme_cli_admin-passthru ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt admin-passthru --op 6 --nsid 1 --dev unvme-ns-0 --cns 2 --array $ARRAYNAME --cdw10 1 --cdw11 2 --cdw12 1 --cdw13 0 --cdw14 0 --cdw15 0 --output admin_passthru.bin --json-res > ${cliOutput}
check_result

echo -[ common: wrong_command_name ]-------------------------------------
${BIN_DIR}/poseidonos-cli wbt wrong_command_name --array $ARRAYNAME --json-res > ${cliOutput}
check_result_expected_fail
echo --------------------------------------------------------------------

${BIN_DIR}/poseidonos-cli wbt flush_gcov
echo "------------[ WBT Test End, Close poseidonos]----------------------------------"
${BIN_DIR}/poseidonos-cli array unmount --array-name $ARRAYNAME --force
${BIN_DIR}/poseidonos-cli system stop --force
