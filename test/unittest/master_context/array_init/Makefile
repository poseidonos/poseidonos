ROOT = ../../../../
GTEST_DIR = $(ROOT)/lib/googletest-release-1.8.1/googletest

INCLUDE = -I$(ROOT) -I$(ROOT)/lib -I$(ROOT)/src/master_context -I$(ROOT)/src/device
TARGET_FILE = $(ROOT)/src/array/array.cpp \
              $(ROOT)/src/array/device/array_device_manager.cpp \
              $(ROOT)/src/array/partition/partition_manager.cpp \
              $(ROOT)/src/array/partition/partition.cpp \
              $(ROOT)/src/array/partition/nvm_partition.cpp \
              $(ROOT)/src/array/partition/stripe_partition.cpp \
              $(ROOT)/src/gc/gc.cpp \
              $(ROOT)/src/array/ft/rebuildable.cpp \
              $(ROOT)/src/array/ft/raid1.cpp \
              $(ROOT)/src/array/ft/raid5.cpp \
              $(ROOT)/src/array/ft/ft_io_entry.cpp \
              $(ROOT)/src/include/buffer.cpp \
              $(ROOT)/src/array/meta/array_meta_manager.cpp \
              $(ROOT)/src/master_context/mbr_manager.cpp \
              $(ROOT)/src/cli/json_helper.cpp \
              $(ROOT)/src/device/mock_device.cpp \
              $(ROOT)/src/device/ublock_device.cpp \
              fake_devicemanager.cpp \
              fake_ubio.cpp \
              fake_event_argument.cpp \
              fake_io_dispatcher.cpp \
              fake_io_worker.cpp \
              fake_io_queue.cpp
UT_FILE = array_init_test.cpp
OUTPUT = array_init_ut
CFLAGS = -g -lpthread -lgtest

all:
	g++ $(INCLUDE) $(UT_FILE) $(TARGET_FILE) $(CFLAGS) -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
