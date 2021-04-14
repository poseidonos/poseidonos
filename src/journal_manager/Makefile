TOP := $(abspath $(CURDIR)/../..)

JM_SUB_DIR= . \
checkpoint \
config \
log \
log_buffer \
log_write \
replay \
statistics \
status
SRCS+=$(wildcard $(foreach dir,$(JM_SUB_DIR),$(dir)/*.cpp))
include $(TOP)/Makefile.rules
