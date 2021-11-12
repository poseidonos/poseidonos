######################################################

IBOFOS_DEP_SRC_INCLUDE=0
ALL_IBOFOS_OBJS_INCLUDE=0
ifeq ($(vmfs),1)
IBOFOS_DEP_SRC_INCLUDE=1
endif
######################################################

IBOF_SRC_DIR := $(abspath $(MFS_TOP_DIR)/../../)
SPDK_ROOT_DIR := $(abspath $(IBOF_SRC_DIR)/lib/spdk)
DPDK_ROOT_DIR := $(abspath $(IBOF_SRC_DIR)/lib/dpdk)
SPDLOG_SOURCE := spdlog-1.4.2
SPDLOG_ROOT_DIR := $(abspath $(IBOF_SRC_DIR)/lib/$(SPDLOG_SOURCE))

VPATH += \
    $(IBOF_SRC_DIR) \
    $(IBOF_SRC_DIR)/src/io \
    $(SPDK_ROOT_DIR)/include \
    $(DPDK_ROOT_DIR)/include/dpdk \
    $(IBOF_SRC_DIR)/src/fault_tolerance \
    $(IBOF_SRC_DIR)/lib/air/src/api \
    $(IBOF_SRC_DIR)/lib/air \
    $(IBOF_SRC_DIR)/lib/config4cpp/include \
    $(IBOF_SRC_DIR)/lib \
    $(IBOF_SRC_DIR)/src/helper \
    $(IBOF_SRC_DIR)/src/logger \
    $(IBOF_SRC_DIR)/src/device \
    $(IBOF_SRC_DIR)/src/master_context \
    $(IBOF_SRC_DIR)/src/lib \
    $(IBOF_SRC_DIR)/src/dump \
    $(IBOF_SRC_DIR)/src/spdk_wrapper \
    $(IBOF_SRC_DIR)/src/cpu_affinity \
    $(SPDLOG_ROOT_DIR)/include

ifneq ($(IBOF_CONFIG_BDEV_FIO_PLUGIN), 1)
    VPATH += $(IBOF_SRC_DIR)/src/io/general_io
endif

ifeq ($(IBOFOS_DEP_SRC_INCLUDE), 1)
SRCS += \
        json_helper.cpp \
        affinity_config_parser.cpp \
        affinity_manager.cpp \
        poverty_cpu_set_generator.cpp \
        cpu_set_generator.cpp \
        count_descripted_cpu_set_generator.cpp \
        string_descripted_cpu_set_generator.cpp \
        pos_event_id.cpp \
        logger.cpp \
        configuration.cpp \
        filter.cpp \
        preferences.cpp \
        default_configuration.cpp \
        config_manager.cpp \
        bitmap.cpp \
        dump_shared_ptr.cpp \
        dump_manager.cpp \
        dump_buffer.cpp
endif

## linking library to use DPDK RTE Ring (should get added even though it is duplicated)
LFLAGS += $(SPDK_ROOT_DIR)/build/lib/libspdk_env_dpdk.a \
           $(DPDK_ROOT_DIR)/lib/librte_eal.a \
           $(DPDK_ROOT_DIR)/lib/librte_ring.a \
           $(DPDK_ROOT_DIR)/lib/librte_mempool_ring.a \
           $(DPDK_ROOT_DIR)/lib/librte_kvargs.a \
           -lnuma -ldl

export ALL_IBOFOS_OBJS_INCLUDE
