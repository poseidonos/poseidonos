TOP := $(abspath $(CURDIR)/../../..)

###################################################
MFS_TOP_DIR :=$(abspath ../../metafs)
include $(MFS_TOP_DIR)/mvm/Makefile.include

###################################################
export UT_TOP_DIR=$(MFS_TOP_DIR)/unit_test
MFS_MVM_DIR:=$(addprefix $(MFS_TOP_DIR)/, $(MFS_INCLUDE_PATH))

UT_INCLUDE =-I$(UT_TOP_DIR) -I$(MFS_MVM_DIR)
MFS_INCLUDE_PATH += \
                    $(UT_INCLUDE) \
                    $(UT_TOP_DIR) \
                    $(MFS_MVM_DIR) 

VPATH += $(MFS_INCLUDE_PATH)
###############################################################

SRCS += $(wildcard *.cpp) \
	meta_file_manager.cpp \
	meta_volume.cpp \
	volume_catalog.cpp \
	volume_catalog_manager.cpp \
	mf_extent.cpp \
	mf_extent_mgr.cpp \
	mf_inode.cpp \
	inode_table_header.cpp \
	mf_inode_mgr.cpp \
	inode_table.cpp \
	on_volume_meta_region_mgr.cpp \
	ssd_meta_volume.cpp \
	nvram_meta_volume.cpp \
	region.cpp \
	region_deque.cpp

UT_SRCS +=\
    mvm_test.cpp \
    mvm_func_negative_test.cpp \
    mvm_func_positive_test.cpp 


SRCS += $(UT_SRCS)

###############################################################
  
-include $(TOP)/Makefile.rules 
include Makefile.mvm
