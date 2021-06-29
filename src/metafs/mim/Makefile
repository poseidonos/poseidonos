TOP := $(abspath $(CURDIR)/../../..)

######################################################################
MFS_TOP_DIR = ../../metafs

include $(MFS_TOP_DIR)/mim/Makefile.include
######################################################################

export UT_TOP_DIR=$(MFS_TOP_DIR)/unit_test
MFS_MIM_DIR:=$(addprefix $(MFS_TOP_DIR)/, $(MFS_INCLUDE_PATH))

UT_INCLUDE=-I$(UT_TOP_DIR) -I$(MFS_MIM_DIR) 
MFS_INCLUDE_PATH += \
                    $(UT_INCLUDE) \
                    $(UT_TOP_DIR) \
                    $(MFS_MIM_DIR) 

VPATH += $(MFS_INCLUDE_PATH) 

######################################################################
SRCS += $(wildcard *.cpp) 
      
#UT_SRCS += \
#           mim_test.cpp \
#           mim_func_positive_test.cpp \
#           mim_func_negative_test.cpp \
#           mim_io_range_overlap_chker_test.cpp \
#           mim_aio_positive_test.cpp 


SRCS += $(UT_SRCS)    

######################################################################

-include $(TOP)/Makefile.rules 
include Makefile.mim
