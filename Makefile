POS_ROOT_DIR := $(abspath $(CURDIR))
SPDK_ROOT_DIR := $(abspath $(CURDIR)/lib/spdk)
SPDLOG_SOURCE := spdlog-1.4.2
SPDLOG_ROOT_DIR := $(abspath $(CURDIR)/lib/$(SPDLOG_SOURCE))

TOP = $(POS_ROOT_DIR)
PROTO_DIR = $(TOP)/proto
PROTO_CPP_GENERATED_DIR = $(PROTO_DIR)/generated/cpp

DPDK_ROOT_DIR := $(abspath $(CURDIR)/lib/dpdk)

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk
include $(SPDK_ROOT_DIR)/mk/spdk.app_vars.mk
include $(POS_ROOT_DIR)/mk/ibof_config.mk

TARGET_SRC_DIR = src fluidos

#################################################
# bin directories / obj files
BINDIR = $(TOP)/bin

#################################################
# docs directories / doc files
DOCDIR = $(TOP)/doc

#################################################
# nvme driver : unvme, libaio

POS_VERSION = v0.11.0-rc0

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

# PoseidonOS CLI
CLI_APP_NAME = poseidonos-cli

CPPFLAGS = -g -Wall -O2 -std=c++14 -Werror
ifeq ($(CONFIG_LIBRARY_BUILD), y)
CPPFLAGS += -fPIC
endif
ifeq ($(CONFIG_FPIC), y)
CPPFLAGS += -fPIC
endif

ifeq ($(CONFIG_GCOV),y)
CPPFLAGS += --coverage
LDFLAGS += -Wl,--dynamic-list-data
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

INCLUDE += -I$(TOP)/lib/air -I$(TOP)/lib/air/src/api
LDFLAGS += -L/usr/local/lib -lair
DEFINE += -DAIR_CFG=$(TOP)/config/air.cfg

INCLUDE += -I$(SPDLOG_ROOT_DIR)/include -I$(SPDLOG_ROOT_DIR)/include/spdlog
LDFLAGS += -L./lib/$(SPDLOG_SOURCE)/lib -lspdlog

CXXFLAGS += $(INCLUDE)

LDFLAGS += -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client
LDFLAGS += -no-pie -laio -ltcmalloc
LDFLAGS += -lnuma
LDFLAGS += -lyaml-cpp
LDFLAGS += -ltbb

CLI_CERT_DIR = /etc/pos/cert
CLI_DIR = $(TOP)/tool/cli
CLI_CERT_FILES = $(CLI_DIR)/cert/cert.key $(CLI_DIR)/cert/cert.crt

# for grpc
LDFLAGS += `pkg-config --libs protobuf grpc++`\
				-lgrpc++_reflection -ldl

LDFLAGS += -lssl -lcrypto


# for callstack symbols
LDEXTRAFLAGS = -rdynamic

CONFIG_DIR = /etc/pos/
CONFIG_FILE = $(CONFIG_DIR)/pos.conf
TELEMETRY_CONFIG_FILE = $(CONFIG_DIR)/telemetry_default.yaml
ifeq ($(CONFIG_LIBRARY_BUILD), y)
LDEXTRAFLAGS += -shared -Wl,-z,nodelete
endif

INSTALL_DIR = /usr/local/bin

UDEV_DIR = /etc/udev/rules.d
UDEV_FILE = $(UDEV_DIR)/99-custom-nvme.rules

################################################

all : $(APP) pos-exporter
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

	@echo "copy default telemetry config file"
	@cp ./config/telemetry_default.yaml ${TELEMETRY_CONFIG_FILE};

#	@echo "make cert dir" \
	@if test -d ${CLI_CERT_DIR}; then \
		echo "exist dir"; \
    else \
		echo "not exist dir"; \
        mkdir -p ${CLI_CERT_DIR};\
		cp ${CLI_CERT_FILES} ${CLI_CERT_DIR}; \
	fi
	@install -m 755 $(APP) $(INSTALL_DIR)
	@install -m 755 $(BINDIR)/${CLI_APP_NAME} $(INSTALL_DIR)

	# Register CLI man page
	gzip --force $(CLI_DIR)/docs/manpage/*
	mv $(CLI_DIR)/docs/manpage/* /usr/share/man/man3/
	makewhatis

udev_install:
	@echo "Try to copy udev bind rule file" 
	@if test -e ${UDEV_FILE}; then \
		echo "The rule file exists"; \
		echo "Updating udev rule file"; \
	else \
		echo "The rule file does not exist"; \
		echo "Copying udev rule file"; \
	fi
	
	$(POS_ROOT_DIR)/tool/udev/generate_udev_rule.sh; \
	cp $(POS_ROOT_DIR)/tool/udev/99-custom-nvme.rules ${UDEV_FILE}; \
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

gen_proto:
	@echo Build protobuf
	@`[ -d $(PROTO_CPP_GENERATED_DIR) ] || mkdir -p $(PROTO_CPP_GENERATED_DIR)`
	protoc --cpp_out=$(PROTO_CPP_GENERATED_DIR) --grpc_out=$(PROTO_CPP_GENERATED_DIR) --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin --proto_path=$(PROTO_DIR) $(PROTO_DIR)/*.proto


pos-exporter:
	@echo Build Telemetry Collector
	$(MAKE) -C tool/pos-exporter
	mv tool/pos-exporter/pos-exporter $(BINDIR)
	@cd $(TOP)

sam: makedir
	@echo SAM Build
	$(MAKE) -C src sam

$(APP) : $(SPDK_LIB_FILES) poseidonos
	$(LINK_CXX) $(shell find src/ -name *.o -and ! -name *_test.o -and ! -name *_fake.o -and ! -name *_stub.o -and ! -name *_mock.o -and ! -name *_fixture.o) $(PROTO_CPP_GENERATED_DIR)/*.o $(LDFLAGS) $(LDEXTRAFLAGS)
	rm bin/ibofos -rf
	ln -s $(shell pwd -P)/bin/poseidonos bin/ibofos

poseidonos: makedir
	$(MAKE) -C proto
	$(MAKE) -C src
	$(CLI_DIR)/script/build_cli.sh
	mv -f $(CLI_DIR)/bin/${CLI_APP_NAME} $(BINDIR)/${CLI_APP_NAME}
	mv -f $(CLI_DIR)/docs/markdown/* $(DOCDIR)/guides/cli/

makedir:
	@`[ -d $(BINDIR) ] || mkdir -p $(BINDIR)`

package: $(APP) pos-exporter
	@$(MAKE) -C package
clean :
	@$(MAKE) -C src clean
	@$(MAKE) -C proto clean
	@$(MAKE) -C package clean
	@rm -rf $(BINDIR)

.PHONY: all install clean udev_install udev_uninstall makedir package

include $(SPDK_ROOT_DIR)/mk/spdk.deps.mk
export INCLUDE SRCS CPPFLAGS DEFINE
