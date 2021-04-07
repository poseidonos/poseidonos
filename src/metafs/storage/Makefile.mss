include $(MFS_TOP_DIR)/storage/Makefile.include

MSS_VSTORE_EN := 0 
MSS_UT_VSTORE_EN := 0

SRCS += \
	mss.cpp \
	mss_utils.cpp \
	mss_status_callback.cpp \

ifeq ($(STANDALONE),1)
	ifeq ($(MFS_INTEGRATION_TEST_EN),0)
		vmfs = 1
	endif
endif
####################
# Virtual MFS 
ifeq ($(vmfs),1)
$(info Virtual Meta Filesystem(Fake module) option enabled...)
MSS_VSTORE_EN = 1
endif

ifeq ($(ut),vmfs)
$(info Virtual meta filesystem(VMFS) unit test option enabled...)
UT_VSTORE_EN = 1
MSS_VSTORE_EN = 1
endif

ifeq ($(MSS_VSTORE_EN), 1)
include ./storage/vstore/Makefile.vstore
else
include ./storage/pstore/Makefile.pstore
endif

SRCS += $(MSS_SRCS)