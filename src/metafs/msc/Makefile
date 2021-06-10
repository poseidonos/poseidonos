TOP := $(abspath $(CURDIR)/../../..)

###############################################################
MFS_TOP_DIR :=$(abspath ../../metafs)
include $(MFS_TOP_DIR)/msc/Makefile.include
###############################################################

export UT_TOP_DIR=$(MFS_TOP_DIR)/unit_test
MFS_MSC_DIR:=$(addprefix $(MFS_TOP_DIR)/, $(MFS_INCLUDE_PATH))

UT_INCLUDE =-I$(UT_TOP_DIR) -I$(MFS_MSC_DIR)
MFS_INCLUDE_PATH += \
                    $(UT_INCLUDE) \
                    $(UT_TOP_DIR) \
                    $(MFS_MSC_DIR) 

VPATH += $(MFS_INCLUDE_PATH)

###############################################################

SRCS += $(wildcard *.cpp) \
	    metafs_mbr.cpp 

SRCS += $(UT_SRCS)
    
###############################################################

-include $(TOP)/Makefile.rules 
include Makefile.msc
