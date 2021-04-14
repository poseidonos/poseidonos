MODULE_DIRS = node

test :
	@for dir in $(MODULE_DIRS); do \
	$(MAKE) -C $$dir || exit $?; \
	done

clean:
	@for dir in $(MODULE_DIRS); do \
	$(MAKE) clean -C $$dir || exit $?; \
	done
