TOP := $(abspath $(CURDIR)/../..)
TELEMETRY_SUB_DIR= . \
grpc_wrapper \
telemetry_air \
telemetry_client

SRCS += $(notdir $(wildcard $(foreach dir, $(TELEMETRY_SUB_DIR), $(dir)/*.cpp)))
VPATH += $(foreach dir, $(TELEMETRY_SUB_DIR), $(abspath $(dir)))
-include $(TOP)/Makefile.rules
