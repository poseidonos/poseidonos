################ MeatFS Vstore & Test Double ##################
# Option 1. MetaFS vstore
include $(MFS_TOP_DIR)/mfs_include.mk
export OBJDIR_MFS=$(UT_OBJDIR)
UT_INCLUDE += $(MFS_INCLUDE_PATH)
MFS_MK_RECIPE=metafs-vstore
#########################################
# Option 2. MetaFS Test double
# please choose test double option
# MFS_EXT_TESTDOUBLE=stub
# MFS_EXT_TESTDOUBLE=mock
# MFS_EXT_TESTDOUBLE=fake
# #----------------------------------------
# export MFS_SLIB_DIR = $(UT_OBJDIR)/metafs
# include $(MFS_TOP_DIR)/mfs_include.mk
# INCLUDE += $(MFS_INCLUDE_PATH)
# MFS_LFLAGS += -L$(MFS_SLIB_DIR) -lmfs-testdouble

# ifeq ($(MFS_MK_RECIPE),mfs-mock)
# MFS_LFLAGS+=-lgmock
# endif

# MFS_MK_RECIPE=mfs-$(MFS_EXT_TESTDOUBLE)
#########################################

export MFS_MK_RECIPE MFS_LFLAGS
