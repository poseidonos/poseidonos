AIR_ROOT_DIR := $(abspath $(CURDIR))

TOP = $(AIR_ROOT_DIR)

BINDIR = $(TOP)/bin
LIBDIR = $(TOP)/lib
INCDIR = $(TOP)/include

OBJDIR = $(BINDIR)/obj
OBJ_FILES = $(OBJDIR)/*.o

OBJDIR_CLI = $(OBJDIR)/cli
OBJ_FILES_CLI = $(OBJDIR_CLI)/*.o

OBJDIR_TUI = $(OBJDIR)/tui
OBJ_FILES_TUI = $(OBJDIR_TUI)/*.o

CXX         = g++
CXXFLAGS    += -std=c++14 -Wall -O3 -mavx2 -fPIC
C			= gcc
CFLAGS		+= -lstdc++ -lm
THREAD      = -lpthread
INCLUDE     = -I$(TOP) -I$(INCDIR)
#DEBUG		= -g
LDFLAGS     += -L$(LIBDIR) -lair

ifndef cfg
DEFINE += -DAIR_CFG=config/air.cfg
CFG_FILE = $(TOP)/config/air.cfg
else
DEFINE += -DAIR_CFG=config/$(cfg).cfg
CFG_FILE = $(TOP)/config/$(cfg).cfg
endif

DEFINE += -DAIR_H_GEN=\"$(TOP)/src/api/Air.h\"
DEFINE += -DAIR_CPP_GEN=\"$(TOP)/src/api/Air.cpp\"

all : release

release : clean lib cli tui

lib : makedir gen_cfg
	@echo [config file]: $(CFG_FILE)
	$(MAKE) -C src
	ar rscv libair.a $(OBJ_FILES)
	mv libair.a $(LIBDIR)
	cp src/api/Air.h $(INCDIR)

makedir :
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(LIBDIR)
	mkdir -p $(INCDIR)

gen_cfg :
	$(MAKE) -C src/config
	$(MAKE) -C tool/cfg_gen
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) -o $(BINDIR)/cfg_gen
	rm -rf $(OBJDIR)/main.o $(OBJDIR)/main.d
	$(TOP)/bin/cfg_gen
	rm -rf $(TOP)/bin/cfg_gen

cli :
	mkdir -p $(OBJDIR_CLI)
	$(MAKE) -C tool/cli
	$(CXX) $(DEBUG) $(OBJ_FILES_CLI) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/air_cli
	cp bin/air_cli /bin/

tui :
	mkdir -p $(OBJDIR_TUI)
	$(MAKE) -C tool/tui
	$(CXX) $(DEBUG) $(OBJ_FILES_TUI) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/air_tui
	cp bin/air_tui /bin/

mock_base : lib cli tui
	$(MAKE) -C test/system_test/mock_base
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/mock_base

st_data_loss_test : lib cli tui
	$(MAKE) -C test/system_test/data_loss_test
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/data_loss_test

st_aid_hash_test : lib cli tui
	$(MAKE) -C test/system_test/aid_hash_test
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/aid_hash_test

st_c_test : lib cli tui
	$(MAKE) -C test/system_test/c_test
	$(C) $(DEBUG) $(OBJ_FILES) $(CFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/c_test

at_rsc_usage_empty : lib
	$(MAKE) -C $(TOP)/test/acceptance_test/resource_usage/empty_node
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/rsc_empty_air

at_rsc_usage_no_air : lib
	$(MAKE) -C $(TOP)/test/acceptance_test/resource_usage/no_air
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/rsc_no_air

at_rsc_usage_perf : lib
	$(MAKE) -C $(TOP)/test/acceptance_test/resource_usage/perf_node
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/rsc_perf_node

at_rsc_usage_lat : lib
	$(MAKE) -C $(TOP)/test/acceptance_test/resource_usage/lat_node
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/rsc_lat_node

at_data_crd : lib
	$(MAKE) -C $(TOP)/test/acceptance_test/data_credibility
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(BINDIR)/data_crd

clean :
	rm -rf $(BINDIR)
	rm -rf $(LIBDIR)
	rm -rf $(INCDIR)
	rm -rf $(TOP)/src/api/Air.h
	rm -rf $(TOP)/src/api/Air.cpp
	rm -rf /bin/air_cli
	rm -rf /bin/air_tui

export TOP OBJS OBJDIR OBJDIR_CLI OBJDIR_TUI BINDIR INCLUDE SRCS CFLAGS CXXFLAGS DEFINE DEBUG
