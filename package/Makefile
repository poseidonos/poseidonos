POS_VERSION := $(shell pwd | sed -n -e '/POS_VERSION =/p' ../Makefile | cut -d 'v' -f2)
PKG_VERSION := $(shell pwd | git log | grep commit | head -1 | cut -d ' ' -f2)

all:
	# Copy library and config files
	@cp -f ../bin/poseidonos src/usr/local/bin/
	@cp -f ../bin/poseidonos-cli src/usr/local/bin/
	@cp -f ../bin/pos-exporter src/usr/local/bin/
	@cp -f ../script/setup_env.sh src/usr/local/etc/
	@cp -rf ../lib/spdk src/usr/local/lib/
	@cp -f ../config/pos.conf src/etc/pos/
	@cp -f ../config/telemetry_default.yaml src/etc/pos/
	
	# Copy CLI man page files
	@gzip --keep ../tool/cli/docs/manpage/*
	@mv ../tool/cli/docs/manpage/*.gz src/etc/
	
	# Package Version Control
	@sed "2s/Version:.*/Version: ${POS_VERSION}-${PKG_VERSION}/g" src/DEBIAN/control -i
	
	# Build Package 
	@`[ -d debian ] || mkdir -p debian`
	@dpkg-deb --build src debian/poseidonos.deb

clean :
	@rm -rf src/usr/local/bin/*
	@rm -rf src/usr/local/lib/*
	@rm -rf src/usr/local/etc/*
	@rm -rf src/etc/pos/*
	@rm -rf src/etc/poseidonos-cli*
	@rm -rf debian/

.PHONY: all clean
