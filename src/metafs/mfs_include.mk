ifeq ($(MFS_TOP_DIR),)
$(error MFS_TOP_DIR not defined...)
endif
#############################################
# DEBUG
MFS_DEBUG = 0
ifeq ($(ut),)
MFS_DEBUG_TRACE=0	# mfs debug trace option
else
MFS_DEBUG_TRACE=1
endif
#############################################

MFS_INCLUDE_PATH += \
	. \
	config \
	log \
	util \
	lib \
	include \
	common

include $(MFS_TOP_DIR)/ext_ibofos_module.mk
include $(MFS_TOP_DIR)/msc/Makefile.include
include $(MFS_TOP_DIR)/mdi/Makefile.include
include $(MFS_TOP_DIR)/mim/Makefile.include
include $(MFS_TOP_DIR)/mvm/Makefile.include
include $(MFS_TOP_DIR)/storage/Makefile.include

DEFINE += "-DMFS_DEBUG_TRACE=$(MFS_DEBUG_TRACE)"
MFS_INCLUDE_PATH := $(addprefix -I${MFS_TOP_DIR}/, $(MFS_INCLUDE_PATH))
MFS_INCLUDE_PATH += $(addprefix -I, ${VPATH})
