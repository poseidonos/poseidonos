#!/bin/bash
cd $(dirname $(realpath $0))

IBOFOS=../../bin/poseidonos
TEMP=collect_binary.tmp
IBOF_CONF=/etc/pos/pos.conf

Branch=`git branch -v | grep "*" | awk '{print $2}'`

Diff=`git diff`

DATE=`date`
echo ""
echo "###############################"
echo "####### Current Time ##########"
echo "###############################"
timedatectl

echo ""
echo "###############################"
echo "######## Config File ##########"
echo "###############################"
if [ -f $IBOF_CONF ]; then
    cat $IBOF_CONF 
    echo ""
else
    echo "There are no pos config file in /etc/pos/"
fi


echo ""
echo "###############################"
echo "###### Build Config ###########"
echo "###############################"

echo ""
cat ../../mk/ibof_config.h
echo ""

echo ""
echo "###############################"
echo "########### Revision ##########"
echo "###############################"
git show | head -2

echo ""
echo "###############################"
echo "### Built Binary Attribute ####"
echo "###############################"
ls -l $IBOFOS

echo ""
echo "###############################"
echo "########### Branch ############"
echo "###############################"
echo -e $Branch
echo ""
echo "###############################"
echo "########### Status ############"
echo "###############################"
git status > $TEMP
cat $TEMP
echo ""
echo "###############################"
echo "########### Diff ##############"
echo "###############################"
if [[ -z "${Diff}" ]];then
    echo "There are no change"
else
    git diff > $TEMP
    cat $TEMP
fi    
rm -rf $TEMP
