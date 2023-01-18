# The purpose of this file is to define spdk/CONFIG file.
# where we can define configurations of Makefile Compilation
# ./configure args will generate mk/ibof_config.mk with default 
# configuration. That file will be used in root Makefile.
# This file can be also used for generating CONFIG variables for any module
# they can define their variables here. And provide options in ./configure

#Build with Metafile system RAID capability (default=RAID1 on)
export CONFIG_MFS_RAID?=n

#Other modules configuration can be defined. eg.
export CONFIG_MOCK_MBR?=n
export CONFIG_MOCK_ARRAY_PARTATION?=n

#Build as fio plugin
export CONFIG_BDEV_FIO_PLUGIN?=n

# Use Mock FileSystem (Linux system default) instead of MFS Layer
export CONFIG_USE_MOCK_FS?=n

# for gcov
export CONFIG_GCOV?=n

# for be qos
export CONFIG_BE_QOS?=y

# for fe qos
export CONFIG_FE_QOS?=n

#Build PoseidonOS as library
export CONFIG_LIBRARY_BUILD?=n

#Build PoseidonOS with -FPIC option
export CONFIG_FPIC?=n

# Build with Address Sanitizer enabled
export CONFIG_ASAN?=n

# Build with Replicator enabled
export CONFIG_REPLICATOR?=n