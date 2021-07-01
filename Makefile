IBOF_ROOT_DIR := $(abspath $(CURDIR))
SPDK_ROOT_DIR := $(abspath $(CURDIR)/lib/spdk)
SPDLOG_SOURCE := spdlog-1.4.2
SPDLOG_ROOT_DIR := $(abspath $(CURDIR)/lib/$(SPDLOG_SOURCE))

TOP = $(IBOF_ROOT_DIR)

DPDK_ROOT_DIR := $(abspath $(CURDIR)/lib/dpdk)

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk
include $(SPDK_ROOT_DIR)/mk/spdk.app_vars.mk
include $(IBOF_ROOT_DIR)/mk/ibof_config.mk

TARGET_SRC_DIR = src fluidos

#################################################
# bin directories / obj files
BINDIR = $(TOP)/bin

#################################################
# nvme driver : unvme, libaio

POS_VERSION = pos-0.9.3

DEFINE += -DPOS_VERSION=\"$(POS_VERSION)\"
DEFINE += -DUNVME_BUILD
DEFINE += -DSPDLOG_COMPILED_LIB

#################################################
# spdk libraries

# SPDK_LIB_LIST to be included in SPDK_LIB_FILES
SPDK_LIB_LIST = $(ALL_MODULES_LIST)

ifeq ($(OS),Linux)
ifeq ($(CONFIG_VHOST),y)
SPDK_LIB_LIST += vhost event_vhost
ifeq ($(CONFIG_VHOST_INTERNAL_LIB),y)
SPDK_LIB_LIST += rte_vhost
endif
endif
endif

SPDK_LIB_LIST += accel event_bdev event_iscsi event_net event_scsi event_nvmf event_vmd event_accel event_sock event
SPDK_LIB_LIST += nvmf trace log conf thread util bdev iscsi scsi rpc jsonrpc json
SPDK_LIB_LIST += net sock notify


ifeq ($(OS),Linux)
SPDK_LIB_LIST += event_nbd nbd
endif

LIBS += $(SPDK_LIB_LINKER_ARGS)

ifeq ($(CONFIG_FC),y)
ifneq ($(strip $(CONFIG_FC_PATH)),)
SYS_LIBS += -L$(CONFIG_FC_PATH)
endif
SYS_LIBS += -lufc
endif

# LINK_CXX includes LIBS 
#LIBS += $(BLOCKDEV_MODULES_LINKER_ARGS) \
#	$(COPY_MODULES_LINKER_ARGS) \
#	$(SOCK_MODULES_LINKER_ARGS) \
#	$(SPDK_LIB_LINKER_ARGS) $(ENV_LINKER_ARGS)

#################################################
ifeq ($(CONFIG_LIBRARY_BUILD), y)
APP = $(BINDIR)/ibofos_library
else
APP = $(BINDIR)/poseidonos
endif

CPPFLAGS = -g -Wall -O2 -std=c++14 -Werror
ifeq ($(CONFIG_LIBRARY_BUILD), y)
CPPFLAGS += -fPIC
endif
ifeq ($(CONFIG_FPIC), y)
CPPFLAGS += -fPIC
endif

ifeq ($(CONFIG_GCOV),y)
CPPFLAGS += --coverage
IBOF_LDFLAGS += -Wl,--dynamic-list-data
endif

# Warning exceptions only for external libraries (dpdk, rapidjson ...)
W_NO = -Wno-register -Wno-class-memaccess
CPPFLAGS += $(W_NO)

KERNEL_VER=$(shell uname -r)
VM_KERNEL_VER=5.3.0-19-generic
ifeq ($(KERNEL_VER), $(VM_KERNEL_VER))
    FIO_SOURCE = fio-fio-3.12
else    
    FIO_SOURCE = fio-fio-3.1
endif       

INCLUDE = -I$(TOP) -I$(SPDK_ROOT_DIR)/include -I$(SPDK_ROOT_DIR)/module -I$(TOP)/lib \
		  -I$(TOP)/src/ibofos/network/ -I$(TOP)/src/logger/
  

export MFS_TOP_DIR=$(TOP)/src/metafs
include $(MFS_TOP_DIR)/mfs_include.mk
INCLUDE += $(MFS_INCLUDE_PATH)

INCLUDE += -I$(DPDK_ROOT_DIR)/include/dpdk

$(info $(INCLUDE))

INCLUDE += -I$(TOP)/lib/air/ -I$(TOP)/lib/air/src/api/
IBOF_LDFLAGS += -L./lib/air/lib/ -lair
DEFINE += -DAIR_CFG=$(TOP)/lib/air/config/air.cfg

INCLUDE += -I$(SPDLOG_ROOT_DIR)/include -I$(SPDLOG_ROOT_DIR)/include/spdlog
IBOF_LDFLAGS += -L./lib/$(SPDLOG_SOURCE)/lib -lspdlog

CXXFLAGS += $(INCLUDE)

IBOF_LDFLAGS += -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client
IBOF_LDFLAGS += -no-pie -laio -ltcmalloc
IBOF_LDFLAGS += -lnuma

CLI_CERT_DIR = /etc/pos/cert
CLI_DIR = $(TOP)/tool/cli
CLI_CERT_FILES = $(CLI_DIR)/cert/cert.key $(CLI_DIR)/cert/cert.crt

IBOF_LDFLAGS += -lssl
IBOF_LDFLAGS += -lcrypto

# for callstack symbols
LDEXTRAFLAGS = -rdynamic

CONFIG_DIR = /etc/pos/
CONFIG_FILE = $(CONFIG_DIR)/pos.conf
ifeq ($(CONFIG_LIBRARY_BUILD), y)
LDEXTRAFLAGS += -shared -Wl,-z,nodelete
endif

INSTALL_DIR = /usr/local/bin

UDEV_DIR = /etc/udev/rules.d
UDEV_FILE = $(UDEV_DIR)/99-custom-nvme.rules

################################################

all : $(APP)
	@:

install: 
	@echo "make config dir"
	@if test -d ${CONFIG_DIR}; then \
		echo "exist dir"; \
    else \
		echo "not exist dir"; \
        mkdir -p ${CONFIG_DIR};\
	fi
	@echo "copy default config file"
	@if test -e ${CONFIG_FILE}; then \
		echo "exist file"; \
        echo "if you want to change default config file, remove file first"; \
	else \
		echo "not exist file"; \
        echo "copy default config file"; \
		cp ./config/pos.conf ${CONFIG_FILE}; \
	fi
	
#	@echo "make cert dir" \
	@if test -d ${CLI_CERT_DIR}; then \
		echo "exist dir"; \
    else \
		echo "not exist dir"; \
        mkdir -p ${CLI_CERT_DIR};\
		cp ${CLI_CERT_FILES} ${CLI_CERT_DIR}; \
	fi
	@install -m 755 $(APP) $(INSTALL_DIR)
	@install -m 755 $(BINDIR)/cli $(INSTALL_DIR)

udev_install:
	@echo "Try to copy udev bind rule file" 
	@if test -e ${UDEV_FILE}; then \
		echo "The rule file exists"; \
		echo "Updating udev rule file"; \
	else \
		echo "The rule file does not exist"; \
		echo "Copying udev rule file"; \
	fi
	
	$(IBOF_ROOT_DIR)/tool/udev/generate_udev_rule.sh; \
	cp $(IBOF_ROOT_DIR)/tool/udev/99-custom-nvme.rules ${UDEV_FILE}; \
	udevadm control --reload-rules && udevadm trigger; \

udev_uninstall:
	@echo "Try to remove udev bind rule file"
	@if test -e ${UDEV_FILE}; then \
		echo "The rule file exists"; \
		rm ${UDEV_FILE};\
		udevadm control --reload-rules && udevadm trigger; \
		echo "The rule file removed"; \
	else \
		echo "The file does not exist"; \
		echo "No need to remove"; \
	fi

sam: makedir
	@echo SAM Build
	$(MAKE) -C src sam

$(APP) : $(SPDK_LIB_FILES) poseidonos
	$(LINK_CXX) $(shell find src/ -name *.o -and ! -name *_test.o -and ! -name *_fake.o -and ! -name *_stub.o -and ! -name *_mock.o -and ! -name *_fixture.o) $(IBOF_LDFLAGS) $(LDEXTRAFLAGS)
	rm bin/ibofos -rf
	ln -s $(shell pwd -P)/bin/poseidonos bin/ibofos

poseidonos: makedir
	$(MAKE) -C src
	$(CLI_DIR)/script/build_cli.sh
	cp -f $(CLI_DIR)/old-cli $(BINDIR)/cli
	mv -f $(CLI_DIR)/bin/poseidonos-cli $(BINDIR)/poseidonos-cli

makedir:
	@`[ -d $(BINDIR) ] || mkdir -p $(BINDIR)`

clean :
	@$(MAKE) -C src clean
	@rm -rf $(BINDIR)

.PHONY: all install clean udev_install udev_uninstall makedir

include $(SPDK_ROOT_DIR)/mk/spdk.deps.mk
export INCLUDE SRCS CPPFLAGS DEFINE
