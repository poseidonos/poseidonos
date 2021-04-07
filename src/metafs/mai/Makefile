TOP := $(abspath $(CURDIR)/../../..)

MFS_TOP_DIR :=$(abspath ../../metafs)
include $(abspath $(MFS_TOP_DIR)/../../mk)/ibof_config.mk


#SRCS += $(wildcard *.cpp)

SRCS += \
	mfs_fscontrol_api.cpp \
	mfs_mgmt_api.cpp \
	mfs_util_api.cpp \
	mfs_io_api.cpp \
	mfs_wbt_api.cpp \

include $(TOP)/Makefile.rules 
