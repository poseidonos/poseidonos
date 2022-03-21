TOP := $(abspath $(CURDIR)/../../..)

MFS_TOP_DIR :=$(abspath ../../metafs)

include $(MFS_TOP_DIR)/storage/Makefile.include

VPATH += $(MFS_INCLUDE_PATH)

export MSS_TOP_DIR=$(MFS_TOP_DIR)/storage

ifneq ($(vmfs),1)
DEFINE += -DWITH_PSTORE_STORAGE
endif


SRCS += \
	mss.cpp \
	mss_utils.cpp \
	mss_status_callback.cpp \
    mss_disk_inplace.cpp \
    mss_disk_place.cpp \
    mss_on_disk.cpp
        
    
include $(TOP)/Makefile.rules 
