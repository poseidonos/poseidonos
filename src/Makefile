export MODULE='"src/"'

SUBDIRS = allocator \
        allocator_service \
        io_scheduler \
        event_scheduler \
        network \
        mapper \
        mapper_service \
        cpu_affinity \
        io/general_io \
        io/frontend_io \
        io/backend_io \
        io/backend_io/rebuild_io \
        io_submit_interface \
        master_context \
        gc \
        gc/flow_control \
        array \
        array/device \
        array/ft \
        array/meta \
        array/partition \
        array/rebuild \
        array/service \
        array/service/io_device_checker \
        array/service/io_locker \
        array/service/io_recover \
        array/service/io_translator \
        array/state \
        array_components \
        array_mgmt \
        rebuild \
        volume \
        device \
        spdk_wrapper \
		spdk_wrapper/caller \
        mbr \
        bio \
        device/unvme \
        device/uram \
        device/base \
        cli \
        lib \
        signal_handler \
        state \
        sys_event \
        main \
        metadata \
        meta_service \
        metafs \
        metafs/common \
        metafs/config \
        metafs/log \
        metafs/util \
        metafs/mai \
        metafs/mdi \
        metafs/mim \
        metafs/msc \
        metafs/mvm \
        metafs/storage \
        logger \
        meta_file_intf \
        journal_manager \
        helper \
        helper/enumerable \
        helper/file \
        helper/json \
        helper/rpc \
        helper/string \
        helper/time \
        sys_info \
        dump \
        qos \
        debug \
        wbt \
        admin \
        resource_manager \
	telemetry \
        telemetry/common \
        telemetry/telemetry_config \
        telemetry/telemetry_config/configs \
        dpdk_wrapper \
        memory_checker

.PHONY: $(SUBDIRS)

poseidonos: $(SUBDIRS)

sam: $(SUBDIRS)

clean: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
