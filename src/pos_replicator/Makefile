TOP := $(abspath $(CURDIR)/../..)

REPLICATOR_SUB_DIR= . \
grpc_service

SRCS+=$(wildcard $(foreach dir,$(REPLICATOR_SUB_DIR),$(dir)/*.cpp))
include $(TOP)/Makefile.rules