
ROOT = ../../../
INCLUDE = -I$(ROOT)
TARGET_FILE = $(ROOT)/src/device/mock_device.cpp fake_ubio.cpp $(ROOT)/src/device/ublock_device.cpp
UT_FILE = mock_device_ut.cpp
OUTPUT = mock_device_ut
CFLAGS = -g -lpthread

all:
	g++ $(INCLUDE) $(UT_FILE) $(TARGET_FILE) $(CFLAGS) -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
