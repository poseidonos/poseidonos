# Makefile for Meta filesystem build

TOP := $(abspath $(CURDIR)/../..)

include tool/Makefile.tool

############################################################
MFS_TOP_DIR=$(CURDIR)
include $(MFS_TOP_DIR)/mfs_include.mk
############################################################
SRCS += $(wildcard *.cpp)

ifeq ($(ut),)
DEFINE += "-DMFS_DEBUG=$(MFS_DEBUG)"
DEFINE += "-DMFS_DEBUG_TRACE=$(MFS_DEBUG_TRACE)"
include $(TOP)/Makefile.rules
else
include Makefile.ut
endif

