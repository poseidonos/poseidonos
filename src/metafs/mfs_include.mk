ifeq ($(MFS_TOP_DIR),)
$(error MFS_TOP_DIR not defined...)
endif
#############################################
COMPACTION_EN=0		# inode compaction option 

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
	common \
	api_base\
    unit_test 


ifdef MFS_EXT_TESTDOUBLE
	TESTDOUBLE_TOP_DIR=testdouble
	DEFINE += -DMFS_EXT_TESTDOUBLE_EN=1
	MFS_INCLUDE_PATH += \
		$(TESTDOUBLE_TOP_DIR) \
		$(TESTDOUBLE_TOP_DIR)/$(MFS_EXT_TESTDOUBLE)

else

include $(MFS_TOP_DIR)/ext_ibofos_module.mk
include $(MFS_TOP_DIR)/mai/Makefile.include
include $(MFS_TOP_DIR)/msc/Makefile.include
include $(MFS_TOP_DIR)/mdi/Makefile.include
include $(MFS_TOP_DIR)/mim/Makefile.include
include $(MFS_TOP_DIR)/mvm/Makefile.include
include $(MFS_TOP_DIR)/storage/Makefile.include

endif

DEFINE += "-DMFS_DEBUG=$(MFS_DEBUG)"
DEFINE += "-DMFS_DEBUG_TRACE=$(MFS_DEBUG_TRACE)"
MFS_INCLUDE_PATH := $(addprefix -I${MFS_TOP_DIR}/, $(MFS_INCLUDE_PATH))
MFS_INCLUDE_PATH += $(addprefix -I, ${VPATH})
