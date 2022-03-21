IBOF_SRC_DIR := $(abspath $(CURDIR)/../../../../)
SPDK_ROOT_DIR := $(abspath $(IBOF_SRC_DIR)/lib/spdk)
DPDK_ROOT_DIR := $(abspath $(IBOF_SRC_DIR)/lib/dpdk)
SRC_DIR := .
MFS_DIR := ./../../

export CONFIG_PSTORE_TEST ?= n
MSS_TARGET := ./pstoreTest

ifeq ($(CONFIG_PSTORE_TEST), y)
OBJ_DIR := $(IBOF_SRC_DIR)/bin/obj
else 
OBJ_DIR := $(SRC_DIR)/obj
endif

gtest := 1
OBJECT_FILES = $(filter-out $(wildcard $(IBOF_SRC_DIR)/bin/obj/main.o) \
				 $(wildcard $(IBOF_SRC_DIR)/bin/obj/fa_cli.o) \
				 $(wildcard $(IBOF_SRC_DIR)/bin/obj/mss_*.o) \
			   , $(wildcard $(IBOF_SRC_DIR)/bin/obj/*.o))

OBJECT_FILES += $(OBJ_DIR)/mss_disk_inplace.o
OBJECT_FILES += $(OBJ_DIR)/mss_status_callback.o
OBJECT_FILES += $(OBJ_DIR)/mss_disk_place.o
OBJECT_FILES += $(OBJ_DIR)/start_ibof.o
OBJECT_FILES +=	$(OBJ_DIR)/mss_on_disk.o

ifeq ($(gtest),1)
OBJECT_FILES +=	$(OBJ_DIR)/pstore_gtest.o
else
OBJECT_FILES +=	$(OBJ_DIR)/unit_test_pstore.o
endif

#spdk libraries, dpdk libraries and other required libraries
PSTORE_LDFLAGS := -DFLUIDOS_CONFIG='"$(IBOF_SRC_DIR)/config/fluidos.cfg"' \
	-lgmock -lgtest \
	-g -Wall -O0 -std=c++14 -Wl,-z,relro,-z,now -Wl,-z,noexecstack \
	-fuse-ld=bfd -pthread \
	-L$(SPDK_ROOT_DIR)/build/lib -Wl,--whole-archive \
	-lspdk_bdev_lvol -lspdk_blobfs 	-lspdk_blob \
	-lspdk_blob_bdev -lspdk_lvol -lspdk_bdev_malloc \
	-lspdk_bdev_null -lspdk_bdev_nvme -lspdk_nvme  \
	-lspdk_bdev_passthru -lspdk_bdev_error -lspdk_bdev_gpt \
	-lspdk_bdev_split -lspdk_bdev_raid -lspdk_bdev_aio \
	-lspdk_bdev_virtio -lspdk_virtio -lspdk_bdev_pos \
	-lspdk_copy_ioat -lspdk_ioat -lspdk_sock_posix \
	-lspdk_event_bdev -lspdk_event_copy -lspdk_event_nvmf \
	-lspdk_event_net -lspdk_nvmf -lspdk_event \
	-lspdk_log -lspdk_trace -lspdk_conf \
	-lspdk_thread -lspdk_util -lspdk_bdev \
	-lspdk_copy -lspdk_rpc -lspdk_jsonrpc \
	-lspdk_json -lspdk_net -lspdk_sock \
	-lspdk_app_rpc -lspdk_log_rpc -lspdk_trace_rpc \
	-lspdk_bdev_rpc -lspdk_event_nbd -lspdk_nbd \
	-Wl,--no-whole-archive \
	$(SPDK_ROOT_DIR)/build/lib/libspdk_env_dpdk.a \
	-Wl,--whole-archive \
	$(DPDK_ROOT_DIR)/lib/librte_eal.a \
	$(DPDK_ROOT_DIR)/lib/librte_mempool.a \
	$(DPDK_ROOT_DIR)/lib/librte_ring.a \
	$(DPDK_ROOT_DIR)/lib/librte_mempool_ring.a \
	$(DPDK_ROOT_DIR)/lib/librte_pci.a \
	$(DPDK_ROOT_DIR)/lib/librte_bus_pci.a \
	$(DPDK_ROOT_DIR)/lib/librte_kvargs.a \
	-Wl,--no-whole-archive \
	-lnuma -ldl -libverbs -lrdmacm -lrt \
	-luuid -lcrypto -libverbs -lrdmacm \
	-laio -no-pie -laio -ludev -lfuse \
	-L$(IBOF_SRC_DIR)/lib/config4cpp/lib -lcpg -lconfig4cpp \


CPPFLAGS := -std=c++14 -g  
CXXFLAGS := -I$(IBOF_SRC_DIR)/src/ibofos/metafs/storage/ \
	-I$(IBOF_SRC_DIR)/src/ibofos/metafs/include \
	-I$(IBOF_SRC_DIR)/src/ibofos/metafs/config \
	-I$(IBOF_SRC_DIR)/ \
	-I$(SRC_DIR)/../ -I$(SRC_DIR)/ -I$(MFS_DIR)/mvm/  \
	-I$(SRC_DIR)/ -I$(MFS_DIR)/config/ -I$(MFS_DIR)/log/ -I$(MFS_DIR)/include/ \
	-I$(MFS_DIR)tore -I$(MFS_DIR)/ -I$(MFS_DIR)/mai/ -I$(MFS_DIR)/mim/ -I$(MFS_DIR)/mdi/ \
	-I$(MFS_DIR)/msc/ -I$(MFS_DIR)/util/ -I$(MFS_DIR)/mdi/mdi_mock \
	-I$(MFS_DIR)/mim/fake -I$(MFS_DIR)/msc/mbr -I$(MFS_DIR)/msc/mock \
	-I$(MFS_DIR)/mvm/mock -I$(MFS_DIR)/mvm/volume -I$(MFS_DIR)/mvm/volume/ssd -I$(MFS_DIR)/mvm/volume/nvram \
	-I$(DPDK_ROOT_DIR)/include/dpdk \
	-I$(SPDK_ROOT_DIR)/include \
	-I$(IBOF_SRC_DIR)/src/fault_tolerance

all: $(OBJ_DIR) $(MSS_TARGET)
	@echo target: $(MSS_TARGET)

ifeq ($(CONFIG_PSTORE_TEST), y)
$(MSS_TARGET) : $(OBJECT_FILES)
	# nothing to do
else 
$(MSS_TARGET) : $(OBJECT_FILES)
	g++ -o $@ $^ $(PSTORE_LDFLAGS)
endif

$(OBJ_DIR)/mss_disk_place.o : $(SRC_DIR)/mss_disk_place.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/mss_status_callback.o : $(SRC_DIR)/mss_status_callback.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/start_ibof.o : $(SRC_DIR)/unit_test/start_ibof.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/mss_disk_inplace.o : $(SRC_DIR)/mss_disk_inplace.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/mss_on_disk.o : $(SRC_DIR)/mss_on_disk.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/pstore_gtest.o : $(SRC_DIR)/unit_test/pstore_gtest.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/unit_test_pstore.o : $(SRC_DIR)/uint_test/unit_test_pstore.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR) :
	mkdir -p $@

clean :
	rm -rf $(OBJ_DIR) $(MSS_TARGET)

ifeq ($(CONFIG_PSTORE_TEST), y)
include $(TOP)/Makefile.rules
endif
