# Makefile for Journal Manager build
#
############################################################

TOP := $(abspath $(CURDIR)/../..)

JM_SUB_DIR= . \
checkpoint \
log \
log_buffer \
log_write \
replay \
unit_test
SRCS+=$(notdir $(wildcard $(foreach dir,$(JM_SUB_DIR),$(dir)/*.cpp)))
VPATH+=$(foreach dir,$(JM_SUB_DIR),$(abspath $(dir)))
-include $(TOP)/Makefile.rules
