OUTPUT = journal_ut
IBOF_DIR = ../../..
SPDLOG_SOURCE = spdlog-1.4.2
SPDLOG_ROOT_DIR = $(abspath $(IBOF_DIR)/lib/$(SPDLOG_SOURCE))

JM_DIR= .. \
../checkpoint \
../config \
../log \
../log_buffer \
../log_write \
../replay \
../statistics \
../status \
../../journal_service


JM_TEST_DIR= . \
mock \
submodule_test \
$(IBOF_DIR)/test/integration-tests/journal \
$(IBOF_DIR)/test/integration-tests/journal/fixture \
$(IBOF_DIR)/test/integration-tests/journal/utils \
$(IBOF_DIR)/test/integration-tests/journal/fake

JOURNAL_SRCS=$(wildcard $(foreach dir,$(JM_DIR),$(dir)/*.cpp))
JOURNAL_TEST_SRCS=$(wildcard $(foreach dir, $(JM_TEST_DIR),$(dir)/*.cpp))

JOURNAL_EXCLUDE = ../../journal_service/journal_service_stub.cpp

TEST_SRCS= \
$(filter-out $(JOURNAL_EXCLUDE),$(JOURNAL_SRCS)) \
$(JOURNAL_TEST_SRCS) \
$(wildcard $(IBOF_DIR)/src/logger/*.cpp) \
$(wildcard $(IBOF_DIR)/src/event_scheduler/*_stub.cpp) \
$(IBOF_DIR)/src/state/state_context_stub.cpp \
$(IBOF_DIR)/src/dump/dump_manager.cpp \
$(IBOF_DIR)/src/dump/dump_shared_ptr.cpp \
$(IBOF_DIR)/src/dump/dump_buffer.cpp \
$(IBOF_DIR)/src/debug/memory_checker.cpp \
$(IBOF_DIR)/src/meta_file_intf/mock_file_intf.cpp \
$(IBOF_DIR)/src/meta_file_intf/meta_file_intf.cpp \
$(IBOF_DIR)/src/meta_file_intf/async_context.cpp \
$(IBOF_DIR)/src/mapper/mapper_stub.cpp \
$(IBOF_DIR)/src/mapper_service/mapper_service_stub.cpp \
$(IBOF_DIR)/src/allocator/allocator_stub.cpp \
$(IBOF_DIR)/src/allocator/block_manager/block_allocator_stub.cpp \
$(IBOF_DIR)/src/allocator/wb_stripe_manager/stripe_stub.cpp \
$(IBOF_DIR)/src/allocator_service/allocator_service_stub.cpp \
$(IBOF_DIR)/src/bio/ubio_stub.cpp \
$(IBOF_DIR)/src/bio/data_buffer_stub.cpp \
$(IBOF_DIR)/src/sys_event/volume_event_stub.cpp \
$(IBOF_DIR)/src/master_context/config_manager.cpp \
$(IBOF_DIR)/src/master_context/default_configuration.cpp \
$(IBOF_DIR)/src/helper/json_helper.cpp

UT_OBJDIR_REL=obj_output
UT_OBJDIR=$(abspath $(UT_OBJDIR_REL))

UT_CPPFLAGS=-g -Wall -Wunused -std=c++14 --coverage -Wfatal-errors -Werror
UT_LFLAGS += -lgtest -lgmock_main -lpthread -lgcov --coverage 
UT_INCLUDE=-I$(IBOF_DIR) -I$(IBOF_DIR)/lib -I$(SPDLOG_ROOT_DIR)/include

TARGET_OBJECTS = $(addprefix $(UT_OBJDIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
VPATH = $(dir $(TEST_SRCS))

ut: $(TARGET_OBJECTS)
	@g++ $(UT_INCLUDE) $(addprefix $(UT_OBJDIR)/, $(notdir $(TARGET_OBJECTS))) $(UT_LFLAGS) -o $(OUTPUT)

$(TARGET_OBJECTS): | configure $(UT_OBJDIR)

$(UT_OBJDIR):
	mkdir -p $@

configure: 
	./$(IBOF_DIR)/configure --with-mockfs

$(UT_OBJDIR)/%.o: %.cpp
	@echo "Compling [$(<F)]..."
	@ccache g++ $(UT_INCLUDE) $(UT_DEFINE) $(UT_CPPFLAGS) -c $< -o $@

clean:
	rm -rf ${OUTPUT} ${UT_OBJDIR} *.bin JournalLogBuffer
	./$(IBOF_DIR)/configure --without-mockfs

reset:
	rm -rf *.bin JournalLogBuffer
