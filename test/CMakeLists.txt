cmake_minimum_required(VERSION 3.10.0)

# https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html

# TODO(yyu): parameterize "-O2" option so that CI could build with -O2
set(CMAKE_CXX_FLAGS "--coverage -ftest-coverage -fpermissive -fPIC")
set(CMAKE_SHARED_LINKER_FLAGS "--coverage -ftest-coverage")

# If needed, we could specify which C++ standard to use.
# POS is currently implemented in C++14. 
set(CMAKE_CXX_STANDARD 14)

# for gdb
set(CMAKE_BUILD_TYPE Debug)

# for the use of ccache (clean build speed up)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

include(GoogleTest)
enable_testing()

get_filename_component(PROJ_ROOT .. ABSOLUTE)
SET(POS_SRC_ROOT ${PROJ_ROOT}/src)
SET(POS_TEST_ROOT ${PROJ_ROOT}/test)

# test build output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${POS_TEST_ROOT}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${POS_TEST_ROOT}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${POS_TEST_ROOT}/build)

MACRO(POS_CMAKE_SUBDIR)
    message("POS_CMAKE_SUBDIR has been deprecated. Skipped.")
ENDMACRO()

MACRO(POS_CMAKE_SUBDIR_ALL)
    message("Including all subdirectories containing CMakeLists.txt")
    # grab all files under test/
    file(GLOB_RECURSE files_cmakelists ${POS_TEST_ROOT}/* LIST_DIRECTORIES true)

    # filter out those under **/CMakeFiles/
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/CMakeFiles/.+")

    # filter out fake directory
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/fake/.+")

    # filter out testdouble directory
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/testdouble/.+")

    # filter out unit tests under src/
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/unit-tests/.+/unit_test/.+")

    # filter out pstore/mock under src/
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/unit-tests/.+/pstore/mock/.+")

    # filter for **/CMakeLists.txt
    list(FILTER files_cmakelists INCLUDE REGEX ".+/CMakeLists.txt")

    # filter out test root directory
    list(FILTER files_cmakelists EXCLUDE REGEX ".+/test/CMakeLists.txt")

    foreach(file_cmakelists ${files_cmakelists})
        #message("file_cmakelists: ${file_cmakelists}")
        get_filename_component(dir_cmakelists "${file_cmakelists}" DIRECTORY)
        message("* Including ${dir_cmakelists}")
        add_subdirectory(${dir_cmakelists})
    endforeach(file_cmakelists ${files_cmakelists})
ENDMACRO()

MACRO(POS_ADD_UNIT_TEST ut_bin ut_testfile)
    message("\t- Populating Makefile(s) for unit test: ${ut_bin}...")
    add_executable(${ut_bin} ${ut_testfile} ${ARGN})
    target_include_directories(${ut_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
    target_link_libraries(${ut_bin} PUBLIC ${DEP_LIBRARIES})
    gtest_add_tests(TARGET ${ut_bin})
ENDMACRO()

MACRO(POS_ADD_INTEGRATION_TEST it_bin it_testfile)
    message("\t- Populating Makefile(s) for integration test: ${it_bin}...")

    message("\t\t- picking up cpp files from ${CMAKE_CURRENT_SOURCE_DIR}/")
    file(GLOB_RECURSE cpps_in_the_same_dir ./*.cpp LIST_DIRECTORIES true)
    list(FILTER cpps_in_the_same_dir EXCLUDE REGEX ".+test.cpp")
    foreach(f ${cpps_in_the_same_dir})
        #message("\t${f}...")
    endforeach(f)

    add_executable(${it_bin} ${it_testfile} ${ARGN} ${cpps_in_the_same_dir})
    target_include_directories(${it_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
    target_link_libraries(${it_bin} PUBLIC ${DEP_LIBRARIES})
    gtest_add_tests(TARGET ${it_bin})
ENDMACRO()

MACRO(POS_ADD_SYSTEM_TEST st_bin st_testfile)
    message("Populating Makefile(s) for system test: ${st_bin}...")

    # It's pretty similar to POS_ADD_UNIT_TEST for now, but may contain more compile & link variations in the future
    add_executable(${st_bin} ${st_testfile} ${ARGN})
    target_include_directories(${st_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
    target_link_libraries(${st_bin} PUBLIC ${DEP_LIBRARIES})
    gtest_add_tests(TARGET ${st_bin})
ENDMACRO()

# Experimental: use this macro only when you want to directly link .o files to 
# your UT binaries (vs. linking with libposlib.a)
MACRO(POS_ADD_TEST_DIRECT ut_bin ut_testfile)
    message("Building ${ut_bin} with objects linked directly...")
    add_executable(${ut_bin} ${ut_testfile} ${ARGN} ${dep_src})
    target_include_directories(${ut_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
    target_link_libraries(${ut_bin} PUBLIC ${DEP_LIBRARIES})
    gtest_add_tests(TARGET ${ut_bin})
ENDMACRO()

MACRO(POS_ASSERT_FILE_EXIST target_file)
    if(EXISTS "${target_file}")
        # do nothing. 
    else()
        message( FATAL_ERROR "${target_file} does not exist. Cannot proceed because the dependency is missing. Quitting the build." )
    endif()
ENDMACRO()

# This is to build libposlib.a
INCLUDE_DIRECTORIES(
    # for POS headers like "./src/include/*.h"
    ${PROJ_ROOT}

    # for "branch_prediction.h"
    ${POS_SRC_ROOT}/include

    # for "i_array_info.h"
    ${POS_SRC_ROOT}/array_models/interface

    # for "metafs.h"
    ${POS_SRC_ROOT}/metafs/include

    # for "metafs_wrapper_api.h"
    ${POS_SRC_ROOT}/metafs/mai

    # for "metafs_adapter_include.h"
    ${POS_SRC_ROOT}/metafs/common

    # for "io_manager_adapter.h"
    ${POS_SRC_ROOT}/metafs/mim

    # for "mfs_lockless_q"
    ${POS_SRC_ROOT}/metafs/lib

    # for "mfs_log.h"
    ${POS_SRC_ROOT}/metafs/log

    # for "mfs_spinlock.h"
    ${PROJ_ROOT}/src/metafs/util

    # for "metafs_config.h"
    ${POS_SRC_ROOT}/metafs/config

    # for "mss.h"
    ${POS_SRC_ROOT}/metafs/storage

    # for "mss_status_callback.h"
    ${POS_SRC_ROOT}/metafs/storage/pstore

    # for "meta_volume_manager.h"
    ${POS_SRC_ROOT}/metafs/mvm

    # for "meta_file_manager.h"
    ${POS_SRC_ROOT}/metafs/mvm/volume

    # for "nvram_meta_volume.h"
    ${POS_SRC_ROOT}/metafs/mvm/volume/nvram

    # for "ssd_meta_volume.h"
    ${POS_SRC_ROOT}/metafs/mvm/volume/ssd

    # for "system_manager_adapter.h"
    ${POS_SRC_ROOT}/metafs/msc

    # for "metafs_mbr.h"
    ${POS_SRC_ROOT}/metafs/msc/mbr

    # for "mdi_manager.h"
    ${POS_SRC_ROOT}/metafs/mdi

    # for "mss_ramdisk.h"
    ${POS_SRC_ROOT}/metafs/storage/vstore

    # for "allocator_address_info.h"
    ${POS_SRC_ROOT}/allocator/address

    # for "e2e_protect.h"
    ${POS_SRC_ROOT}/master_context

    # for _ADMIN_ENABLED flag: "spdk/include/spdk/nvme_spec.h"
    ${PROJ_ROOT}/lib

    # for rte_config.h. 
    ${PROJ_ROOT}/lib/dpdk/include/dpdk
    ${PROJ_ROOT}/lib/dpdk-20.08/config
    # "meson build" needs to run at dpdk dir
    ${PROJ_ROOT}/lib/dpdk-20.08/build

    # for IO_TYPE::FLUSH
    ${PROJ_ROOT}/lib/spdk/include

    # for accel_engine_ioat.h
    ${PROJ_ROOT}/lib/spdk/module

    # for "nlohmann/json.hpp" + some resolve by <> instead of "", so need symlink at src/include
    ${PROJ_ROOT}/lib/nlohmann

    # for <numa.h> that includes numa_node_of_cpu() macro
    /usr

    # for AIRLOG() macro
    ${PROJ_ROOT}/lib/air

    # for enum AIR_NODE like Q_EVENT
    ${PROJ_ROOT}/lib/air/src/api

    ${PROJ_ROOT}/lib/spdlog-1.4.2/include
)

# This is to provide "-I" option for unit test file(s) build
SET(POS_UT_INCLUDE_DIRS
    ${PROJ_ROOT}
)

# This is to provide "-I" option for unit test file(s) build
SET(POS_UT_INCLUDE_DIRS
    ${PROJ_ROOT}
)

message("Creating nlohmann/json symlink to support both \"nlohmann/json\" and <nlohmann/json>")
add_custom_target(nlohmann_json ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJ_ROOT}/lib/nlohmann ${PROJ_ROOT}/src/include/nlohmann)

# Run mock/test generator with cpp_formatter.py automatically
#message("Running mock/test-skel generator (but without cpp_formatter.py)...")
#execute_process(COMMAND ./generate_mocks.sh) # commented out until we figure out how to integrate git-clang-format with UT build

# Build gtest conditionally
SET(POS_DEP_GTEST ${PROJ_ROOT}/lib/googletest-release-1.10.x)
if(EXISTS "${POS_DEP_GTEST}")
    message("GTEST build exists: ${POS_DEP_GTEST}. Build skipped.")
else()
    message("Building GTEST using 'build_ibof_lib.sh gtest'...")
    execute_process(COMMAND ../lib/build_ibof_lib.sh gtest)
    POS_ASSERT_FILE_EXIST("${POS_DEP_GTEST}")
endif()

# Build SPDLOG conditionally
SET(POS_DEP_SPDLOG ${PROJ_ROOT}/lib/spdlog-1.4.2/lib/libspdlog.a)
if(EXISTS "${POS_DEP_SPDLOG}")
    message("SPDLOG build exists: ${POS_DEP_SPDLOG}. Build skipped.")
else()
    message("Building SPDLOG using 'build_ibof_lib.sh spdlog'...")
    execute_process(COMMAND ../lib/build_ibof_lib.sh spdlog)
    POS_ASSERT_FILE_EXIST("${POS_DEP_SPDLOG}")
endif()

# Build DPDK conditionally
SET(POS_DEP_DPDK ${PROJ_ROOT}/lib/dpdk-20.08/build)
if(EXISTS "${POS_DEP_DPDK}")
    message("DPDK build exists: ${POS_DEP_DPDK}. Build skipped.")
else()
    message("DPDK needs to be built")
    message("Building DPDK and generating rte_build_config.h ...")
    execute_process(COMMAND ./build_dpdk.sh)
    POS_ASSERT_FILE_EXIST("${POS_DEP_DPDK}")
    #execute_process(COMMAND ./build_dpdk.sh OUTPUT_VARIABLE cmd_out RESULT_VARIABLE res_var)
    #message("cmd: ${cmd_out}")
endif()

# Build SPDK conditionally
SET(POS_DEP_SPDK ${PROJ_ROOT}/lib/spdk/build/lib)
if(EXISTS "${POS_DEP_SPDK}")
    message("SPDK library exists: ${POS_DEP_SPDK}. Build skipped.")
else()
    message("SPDK library needs to be built")
    execute_process(COMMAND ../lib/build_ibof_lib.sh spdk)
    POS_ASSERT_FILE_EXIST("${POS_DEP_SPDK}")
endif()

# Build AIR conditionally
SET(POS_DEP_AIR "${PROJ_ROOT}/lib/air/lib/libair.a")
if(EXISTS "${POS_DEP_AIR}")
    message("AIR library exists: ${POS_DEP_AIR}. Build skipped.")
else()
    message("AIR library needs to be built.")
    execute_process(COMMAND ../lib/build_ibof_lib.sh air)
    POS_ASSERT_FILE_EXIST("${POS_DEP_AIR}")
endif()

#maybe we could embed likely() unlikely() macro into add_definitions() for unit-test's build context
#commented out UNVME_BUILD since this requires rte_malloc() to function properly, which is meaningful 
#only in prod release build, but not in unit test build
add_definitions(-DQOS_ENABLED_BE 
    -DPOS_VERSION="pos-0.8.0-alpha1" 
    #-DUNVME_BUILD 
    -DSPDLOG_COMPILED_LIB
    -D_ADMIN_ENABLED
)
remove_definitions(-DAIR_BUILD)

#file(GLOB_RECURSE dep_src ${POS_SRC_ROOT} LIST_DIRECTORIES true ${POS_SRC_ROOT} *.cpp)
file(GLOB_RECURSE dep_src ${POS_SRC_ROOT}/*.cpp LIST_DIRECTORIES true)
list(FILTER dep_src EXCLUDE REGEX ".+test.cpp")
list(FILTER dep_src EXCLUDE REGEX ".+fake.cpp")
list(FILTER dep_src EXCLUDE REGEX ".+stub.cpp")
list(FILTER dep_src EXCLUDE REGEX ".+mock.cpp")
list(FILTER dep_src EXCLUDE REGEX ".+/unit_test/.+")
list(FILTER dep_src EXCLUDE REGEX ".+/fake/.+")
list(FILTER dep_src EXCLUDE REGEX ".+/mock/.+")

foreach(f ${dep_src})
    #message("picking up ${f} for compile")
endforeach(f)

# Tried with OBJECT library described at https://stackoverflow.com/questions/1388877/how-to-build-object-files-only-once-with-cmake
# but ran into sigkill when compiling & linking UT binaries (cyclic dep?). Let's use shared lib for now since it's smaller and a bit faster than static build/link. 
add_library(
    poslib SHARED ${dep_src} 
    #poslib STATIC ${dep_src}
)

file(GLOB_RECURSE fixture_src ${POS_TEST_ROOT}/fixtures/*.cpp LIST_DIRECTORIES true)
file(GLOB_RECURSE utils_src ${POS_TEST_ROOT}/utils/*.cpp LIST_DIRECTORIES true)

# for fixtures and utils
add_library(
    postestlib STATIC ${fixture_src} ${utils_src}
)

link_directories (
    ${PROJ_ROOT}/lib/dpdk/lib
    ${PROJ_ROOT}/lib/air/lib
    ${PROJ_ROOT}/lib/spdk/build/lib

    # for libspdlog.a
    ${PROJ_ROOT}/lib/spdlog-1.4.2/lib

    # for libuuid.a, libdl.a
    /usr/lib/x86_64-linux-gnu

    /usr/local/lib
)

# The order of libraries seems to matter
SET(DEP_LIBRARIES 
    gtest
    gmock_main
    gmock
    poslib
    postestlib
    spdk_thread
    spdk_event
    spdk_json
    spdk_jsonrpc
    spdk_rpc
    spdk_trace
    spdk_nvme
    spdk_nvmf
    spdk_event_nvmf
    spdk_bdev
    spdk_notify
    spdk_ioat
    spdk_bdev_ibof
    spdk_bdev_malloc
    spdk_env_dpdk
    spdk_conf
    spdk_accel
    spdk_accel_ioat
    spdk_ftl
    spdk_util
    uuid
    rt
    spdk_rdma
    spdk_log
    spdlog
    dpdk
    spdk_ioat
    air
    numa
    dl
    pthread
)

# For on-demand execution
add_custom_target(
    gen_ut
    COMMAND cd ${POS_TEST_ROOT}/generator && ./generator.py ${POS_SRC_ROOT} && ../../script/cpp_formatter.py -d ../unit-tests
)

# Run code-coverage on-demand (COMMAND mode)
add_custom_target(
    run_cov COMMAND ./report_coverage.sh
)

add_custom_target(
    clean_cov
    COMMAND find -name "*.gcda" -exec rm -f {} \; VERBATIM
)

# -T allows to generate the test results in xml
add_custom_target(
    run_basic_tests COMMAND ctest -T Test -E "SystemTest"
)

add_custom_target(
    run_full_tests COMMAND ctest -T Test
)

add_custom_target(
    run_st COMMAND ctest -T Test -R "SystemTest"
)

add_custom_target(
    run_it COMMAND ctest -T Test -R "IntegrationTest"
)

add_custom_target(
    run_ut COMMAND ctest -T Test -E "SystemTest\\\|IntegrationTest\\\|VolumeTest"
)

POS_CMAKE_SUBDIR_ALL()
