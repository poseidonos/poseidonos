TOP := $(abspath $(CURDIR)/../..)
MFS_TOP_DIR=$(CURDIR)

#############################################
# DEBUG
MFS_DEBUG_TRACE=0	# mfs debug trace option
#############################################

MFS_INCLUDE_PATH += \
	. \
	config \
	log \
	util \
	lib \
	include \
	common

include tool/Makefile.tool
include $(MFS_TOP_DIR)/mai/Makefile.include
include $(MFS_TOP_DIR)/msc/Makefile.include
include $(MFS_TOP_DIR)/mdi/Makefile.include
include $(MFS_TOP_DIR)/mim/Makefile.include
include $(MFS_TOP_DIR)/mvm/Makefile.include
include $(MFS_TOP_DIR)/storage/Makefile.include

DEFINE += "-DMFS_DEBUG_TRACE=$(MFS_DEBUG_TRACE)"
MFS_INCLUDE_PATH := $(addprefix -I${MFS_TOP_DIR}/, $(MFS_INCLUDE_PATH))
MFS_INCLUDE_PATH += $(addprefix -I, ${VPATH})

############################################################
SRCS += $(wildcard *.cpp)

include $(TOP)/Makefile.rules
