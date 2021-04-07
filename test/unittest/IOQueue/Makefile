ROOT = ../../../
TARGET_FILE = ${ROOT}src/ibofos/scheduler/IOQueue.cpp 
UT_FILE = IOQueue_unittest.cpp
CFLAGS = -lgtest -lgtest_main -lpthread
OUTPUT = IOQueue_unittest
all:
	g++ -I${ROOT} ${TARGET_FILE} ${UT_FILE} ${CFLAGS} -o ${OUTPUT}
clean:
	rm -rf ${OUTPUT}
