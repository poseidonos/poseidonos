export MODULE='"src/"'

SUBDIRS = allocator \
        scheduler \
        network \
        mapper \
        io/general_io \
        io/frontend_io \
        io/backend_io \
        master_context \
        gc \
        array \
        array/device \
        array/ft \
        array/meta \
        array/partition \
        volume \
        device \
        device/spdk \
        device/unvme \
        device/unvram \
        cli \
        lib \
        state \
        sys_event \
        main \
        metafs \
        metafs/common \
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
        sys_info \
        dump \
        qos \
        debug \
        wbt

.PHONY: $(SUBDIRS)

ibofos: $(SUBDIRS)

sam: $(SUBDIRS)

clean: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
