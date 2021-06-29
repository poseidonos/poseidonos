perf ?= 0
LDFLAGS = -g
CPPFLAGS := -g -std=c++11 -Wall
INCLUDES := -I./../ -I./../../config -I./../../log/ -I./../../include -I./ -I./../../mvm/ -I./../../log/

OBJECT_FILES = ./obj/mss.o
OBJECT_FILES += ./obj/mss_utils.o
OBJECT_FILES += ./obj/mfs_log.o
OBJECT_FILES += ./obj/mss_ramdisk.o
ifeq ($(perf),0)
OBJECT_FILES += ./obj/ut_mss_ramdisk.o
else 
OBJECT_FILES += ./obj/performance.o
endif

ramdisk: $(OBJECT_FILES)
	g++ $(LDFLAGS) -o $@ $^

./obj/mss.o : ./../mss.cpp
	g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

./obj/mss_utils.o : ./../mss_utils.cpp
	g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

./obj/metafs_log.o : ./../../log/metafs_log.cpp
	g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

./obj/mss_ramdisk.o : mss_ramdisk.cpp
	g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

#./obj/ut_mss_ramdisk.o : unit_test/ut_mss_ramdisk.cpp
#	g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

#./obj/performance.o : unit_test/performance.cpp
#	 g++ $(INCLUDES) $(CPPFLAGS) -c -o $@ $^

clean:
	rm -f obj/* ramdisk 
