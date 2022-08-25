TOP := $(abspath $(CURDIR)/../..)

REPLICATOR_SUB_DIR= . \
dummy_ha \
grpc_service

SRCS+=$(wildcard $(foreach dir,$(REPLICATOR_SUB_DIR),$(dir)/*.cpp))
include $(TOP)/Makefile.rules