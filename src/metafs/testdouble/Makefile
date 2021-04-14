IBOF_TOP_DIR=../../..
#############################################
ifndef MFS_TOP_DIR
$(info MFS_TOP_DIR is not given. implicitly set to $(abspath ../))
MFS_TOP_DIR=../
endif

ifdef MFS_SLIB_DIR
TARGET_SLIB=libmfs-testdouble.a
else
$(info MFS_SLIB_DIR is not given...implicitly set to $(CURDIR)/lib)
MFS_SLIB_DIR=$(CURDIR)/lib
TARGET_SLIB=libmfs-testdouble.a
endif

#############################################
MFS_EXTERNAL_MOCK=0
MFS_EXTERNAL_STUB=0
MFS_EXTERNAL_FAKE=0

#############################################
# ibofOS target code build
OBJDIR:=$(CURDIR)/obj_output

ifeq ($(MAKECMDGOALS), mock)
  $(info Build metafs test double: Mock for external Use)
  MFS_EXTERNAL_MOCK=1

else ifeq ($(MAKECMDGOALS), stub)
  $(info Build metafs test double: Stub for external Use)
  MFS_EXTERNAL_STUB=1

else ifeq ($(MAKECMDGOALS), fake)
  $(info Build metafs test double: Fake for external Use)
  MFS_EXTERNAL_FAKE=1
else ifeq ($(MAKECMDGOALS), clean)
else
  $(error testdouble target is not given (stub|mock|fake))  
endif

#########################################
# define path variables
VPATH := 
UT_PATH := 
# define source variables
SRCS :=
UT_SRCS :=

DEFINE += -DMFS_EXT_TESTDOUBLE_EN=1
#########################################################

ifeq ($(MFS_EXTERNAL_STUB),1)
	MFS_INCLUDE_PATH += stub
else ifeq ($(MFS_EXTERNAL_MOCK),1)
	MFS_INCLUDE_PATH += mock
else ifeq  ($(MFS_EXTERNAL_FAKE),1)
	MFS_INCLUDE_PATH += fake
endif

include Makefile.testdouble
#########################################################
include $(MFS_TOP_DIR)/Makefile.top
#########################################################
$(info -------------------------------------)
UT_PATH += unit_test

MFS_INCLUDE_PATH += \
	$(IBOF_TOP_DIR) \
	$(MFS_TOP_DIR)/. \
	$(MFS_TOP_DIR)/config \
	$(MFS_TOP_DIR)/log \
	$(MFS_TOP_DIR)/util \
	$(MFS_TOP_DIR)/include \
	$(MFS_TOP_DIR)/common \
	$(MFS_TOP_DIR)/api_base \
	$(MFS_TOP_DIR)/testdouble \
	$(MFS_TOP_DIR)/testdouble/$(MAKECMDGOALS)

VPATH += \
	$(MFS_INCLUDE_PATH)

ifeq ($(MFS_EXTERNAL_MOCK),1)
LFLAGS += -lgmock
endif
CC := g++
CPPFLAGS := -std=c++14 -Wall

ifeq ($(MFS_DEBUG),1)
CPPFLAGS += -g -O0
else
CPPFLAGS += -g -O2 -finline-functions
endif

#########################################################
SRCS += $(UT_SRCS)
VPATH += $(UT_PATH)
OBJS += $(SRCS:.cpp=.o)

# recipes
_OBJS=$(addprefix $(OBJDIR)/, $(OBJS))

# build mfs obj only and provide it to ibofos build
stub: $(OBJDIR) $(TARGET_SLIB)
mock: $(OBJDIR) $(TARGET_SLIB)
fake: $(OBJDIR) $(TARGET_SLIB)

$(TARGET): $(_OBJS)
	@$(CC) $(CPPFLAGS) -o $(TARGET) $(_OBJS) $(STANDALONE_OBJS) $(LFLAGS)
	$(info target: $(TARGET))

$(TARGET_SLIB): $(MFS_SLIB_DIR) $(_OBJS)
	@ar -rv $(MFS_SLIB_DIR)/$(TARGET_SLIB) $(_OBJS)
	@echo "Linking: $(MFS_SLIB_DIR)/$(TARGET_SLIB)"

$(OBJDIR)/%.o: %.cpp
	@echo "Compiling [$(<F)]..."
	@$(CC) $(CPPFLAGS) $(addprefix -I, $(VPATH)) $(DEFINE) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

$(MFS_SLIB_DIR):
	mkdir -p $@

clean:
	@echo "Clean up..."
	rm -rf $(OBJDIR) $(TARGET)
	rm -rf $(MFS_SLIB_DIR)