# Meta filesystem Test Double 

MetaFS test double provides a set of testability for external modules

# How to interconnect with specific MetaFS test double object
In your Makefile, add lines specified below:

Note: do not change variable name being used below

    MFS_EXT_TESTDOUBLE=fake|stub|mock

    export MFS_SLIB_DIR = $(OBJDIR)/metafs
    export MFS_TOP_DIR=$(IBOF_DIR)/src/metafs    
    include $(MFS_TOP_DIR)/mfs_include.mk
    INCLUDE += $(MFS_INCLUDE_PATH)

Please refer to following sample. You can simply include this file in order to utilize full MetaFS test suite capability (i.e. MetaFS vstore & Test double)

    ./src/metafs/metafs-testsuite.mk

And add below in your Makefile for MetaFS test double build:

    make -C src/metafs/testdouble $(MFS_EXT_TESTDOUBLE) -j16

Then, link metafs test double library to the target & do final build:

    LFLAGS += -L$(MFS_SLIB_DIR) -lmfs-testdouble

# How to clean test double objects    

    make -C src/metafs/testdouble clean

# MetaFS Test Double Details
## Stub
 - Return always success whenever you call metafs API
 - Thus, stub is not capable of full ibofos integration test and applicable to only specific unit test

## Mock
 - MetaFS Mock is implemented adapting Google Mock framework
 - Mock object allows you to do better unit test of your functions leveraging Google GMock suite
 
## Fake
 - Provides fake-level functionality that metafs stores all files into OS filesystem other than PoseidonOS partitions
 - Fake is capable of full ibofos integration test in a case that product code of metafs doesn't work properly
