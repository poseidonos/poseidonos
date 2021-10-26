TOP := $(abspath $(CURDIR)/../..)

ALLOCATOR_SUB_DIR= . \
address \
block_manager \
context_manager \
context_manager/gc_ctx \
context_manager/io_ctx \
context_manager/allocator_ctx \
context_manager/rebuild_ctx \
context_manager/segment_ctx \
event \
stripe \
wbstripe_manager

SRCS += $(notdir $(wildcard $(foreach dir, $(ALLOCATOR_SUB_DIR), $(dir)/*.cpp)))
VPATH += $(foreach dir, $(ALLOCATOR_SUB_DIR), $(abspath $(dir)))
-include $(TOP)/Makefile.rules
