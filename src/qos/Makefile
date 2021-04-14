TOP := $(abspath $(CURDIR)/../..)

ifeq ($(CONFIG_FE_QOS),y)
$(info FE QOS Enabled,  BE QOS Enabled)
SRCS += $(wildcard *.cpp)
else ifeq ($(CONFIG_BE_QOS),y)
$(info FE QOS Disabled, BE QOS Enabled)
SRCS += $(wildcard *.cpp)
else
$(info FE QOS Disabled, BE QOS Disabled)
endif

include $(TOP)/Makefile.rules
