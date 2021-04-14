#!/bin/sh

# please configure below for your env.
sam_path=/home/sangphil/util/SAM-Tools-SAM_v2.1-rc5
#sam_path=/home/sangphil/util/SAM-Tools-SAM_v2.1-rc1

echo "Usage: ./script/run_sam.sh <directory>"

##----------------------------------------------------
if [ -z $1 ]; then
    echo "please input target directory to run SAM...";
fi
cwd=$1

echo "Please check given configuration below:"
echo "---------------------------------------"
echo "SAM path=${sam_path}"
echo "  - Please make sure you configured sam_cli.cfg properly!"
echo "Target directory path=`realpath $1`"
echo "---------------------------------------"
read -p "Press any key to proceed..." x
cd ${cwd}
${sam_path}/sam_cli.sh ${sam_path}/sam_cli.cfg
# clean up
rm -rf .scap/ .scra-ir/
cd -
