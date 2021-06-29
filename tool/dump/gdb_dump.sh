#please enter $1 as pid or [corefile name], $2 as gdb script
if [ "$2" = "enable" ];then

gdb ../../bin/poseidonos --batch $1 -ex 'p 'pos::$3'.isEnabled=1'

elif [ "$2" = "disable" ];then

gdb ../../bin/poseidonos --batch $1 -ex 'p 'pos::$3'.isEnabled=0'

elif [ "$2" = "object" ];then

gdb ../../bin/poseidonos --batch $1 -ex 'p 'pos::$3

elif [ "$2" = "buffer" ];then

gdb ../../bin/poseidonos --batch $1 -ex 'x/'$4' '$3

elif [ "$2" = "coredump" ];then

gdb ../../bin/poseidonos --batch $1 -ex 'generate-core-file'

else

echo "====GDB script for Dump Module===="
echo "./gdb.sh [process id or core file name] [option] [param...]"
echo " "
echo "option 1 : enable the dump"
echo "./gdb.sh [process id] enable [module varible name]"
echo "example # ./gdb.sh 24500 enable dumpMemDump"
echo " "
echo "option 2 : disable the dump"
echo "./gdb.sh [process id] disable [module varible name]"
echo "example # ./gdb.sh 24500 disable dumpMemDump"
echo " "
echo "option 3 : show the dump object"
echo "./gdb.sh [process id or core file name] object [module varible name]"
echo "example # ./gdb.sh 24500 object dumpMemDump"
echo " "
echo "option 4 : see the raw buffer "
echo "./gdb.sh [process id or core file name] enable [base addr] [size]"
echo "example # ./gdb.sh 24500 buffer 0xffffff40 512"
echo " "
echo "option 5 : get the core dump "
echo "example # ./gdb.sh [process id] coredump"
echo " "

fi
