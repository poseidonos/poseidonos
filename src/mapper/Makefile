TOP := $(abspath $(CURDIR)/../..)
MAPPER_SUB_DIR= . \
address \
map \
reversemap \
stripemap \
vsamap

SRCS += $(notdir $(wildcard $(foreach dir, $(MAPPER_SUB_DIR), $(dir)/*.cpp)))
VPATH += $(foreach dir, $(MAPPER_SUB_DIR), $(abspath $(dir)))
-include $(TOP)/Makefile.rules
