cmake_minimum_required(VERSION 3.10.0)

# https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html

# Default options
option(POS_BUILD_LARGE_UT "Coalesce all UTs/ITs into two binaries" ON)
option(POS_PRESERVE_TEST_DIR "Build test binaries at `test/build/` while preserving their original dir hierarchy (works only when POS_BUILD_LARGE_UT is OFF)" OFF)
option(POS_BUILD_WITH_O2 "Use gcc O2 optimization to be on par with prod build" OFF)
option(POS_SKIP_UT_BUILD "Skip compiling test/unit-tests/**/*_test.cpp" OFF)
option(POS_SKIP_IT_BUILD "Skip compiling test/integration-tests/**/*_test.cpp" OFF)
option(POS_BUILD_SHARED_LIB "Build POS as shared library (vs. static)" ON)
option(POS_TESTEXEC_WITH_NATIVE_GTEST "Run UTs/ITs by executing gtest binaries directly (vs. ctest)" ON)
option(POS_DISABLE_LOGGING "Disable the POS logger while running UTs/ITs" ON)

# By default, we use 4 threads to run unit tests and 1 thread to run integration tests.
# Those numbers are to limit the tests from consuming too much resource of a build server, 
# but may not be relevant to users' perf requirement. You could either optimize test execution times
# by increasing the parallelism, or throttle by decreasing the value.
set(POS_PARALLEL_EXEC_UT "4" CACHE STRING "Specify the number of parallel threads to run unit tests")
set(POS_PARALLEL_EXEC_IT "1" CACHE STRING "Specify the number of parallel threads to run integration tests")

set(POS_USE_LOCAL_REPO "n" CACHE STRING "Specify use of local repository, y or n")

set(CMAKE_CXX_FLAGS "--coverage -ftest-coverage -fpermissive -fPIC -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "--coverage -ftest-coverage")

if(POS_BUILD_WITH_O2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif(POS_BUILD_WITH_O2)

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

# ${test_type} should be either unit-tests or integration-tests
MACRO(POS_MOVE_BINARIES_IFNEEDED target_bin target_testfile test_type)
    if(POS_PRESERVE_TEST_DIR)
        set(target_testfile_dir ${CMAKE_CURRENT_SOURCE_DIR})
        string(REPLACE ${POS_TEST_ROOT}/${test_type}/ "" dir_without_proj_prefix ${target_testfile_dir})

        set(TARGET_DIR ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${test_type}/${dir_without_proj_prefix})
        add_custom_command(TARGET ${target_bin} POST_BUILD 
            COMMAND mkdir -p ${TARGET_DIR}
                && mv ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${target_bin} ${TARGET_DIR})
    endif()
ENDMACRO()

MACRO(POS_ADD_UNIT_TEST ut_bin ut_testfile)
    if(POS_SKIP_UT_BUILD)
        return()
    endif(POS_SKIP_UT_BUILD)

    message("\t- Populating Makefile(s) for unit test: ${ut_bin} at ${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(ut_testfile_fullpath ${ut_testfile} ABSOLUTE)
    if(POS_BUILD_LARGE_UT)
        # just append to the global list and build at the last stage
        list(APPEND ut_test_files ${ut_testfile_fullpath})
        set(ut_test_files ${ut_test_files} PARENT_SCOPE)
    else()
        # add to gtest immediately
        add_executable(${ut_bin} ${ut_testfile} ${ARGN})
        target_include_directories(${ut_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
        target_link_libraries(${ut_bin} PUBLIC ${DEP_LIBRARIES})
        gtest_add_tests(TARGET ${ut_bin})
        POS_MOVE_BINARIES_IFNEEDED(${ut_bin} ${ut_testfile} "unit-tests")

    endif(POS_BUILD_LARGE_UT)
ENDMACRO()

MACRO(POS_ADD_INTEGRATION_TEST it_bin it_testfile)
    if(POS_SKIP_IT_BUILD)
        return()
    endif(POS_SKIP_IT_BUILD)

    message("\t- Populating Makefile(s) for integration test: ${it_bin}...")

    message("\t\t- picking up cpp files from ${CMAKE_CURRENT_SOURCE_DIR}/")
    file(GLOB_RECURSE cpps_in_the_same_dir ./*.cpp LIST_DIRECTORIES true)
    list(FILTER cpps_in_the_same_dir EXCLUDE REGEX ".+test.cpp")
    foreach(f ${cpps_in_the_same_dir})
        #message("\t${f}...")
    endforeach(f)
    if(POS_BUILD_LARGE_UT)
        # just append to the global list and build at the last stage
        get_filename_component(it_testfile_fullpath ${it_testfile} ABSOLUTE)
        list(APPEND it_test_files ${it_testfile_fullpath} ${cpps_in_the_same_dir})
        set(it_test_files ${it_test_files} PARENT_SCOPE)
    else()
        # add to gtest immediately
        add_executable(${it_bin} ${it_testfile} ${ARGN} ${cpps_in_the_same_dir})
        target_include_directories(${it_bin} PUBLIC ${POS_UT_INCLUDE_DIRS})
        target_link_libraries(${it_bin} PUBLIC ${DEP_LIBRARIES})
        gtest_add_tests(TARGET ${it_bin})

        POS_MOVE_BINARIES_IFNEEDED(${it_bin} ${it_testfile} "integration-tests")
    endif(POS_BUILD_LARGE_UT)
ENDMACRO()

MACRO(POS_ADD_SYSTEM_TEST st_bin st_testfile)
    message("Not supported at the moment: ${st_bin}...")
    # It's pretty similar to POS_ADD_UNIT_TEST for now, but may contain more compile & link variations in the future
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

    # for "mss_aio_cb_cxt.h"
    ${POS_SRC_ROOT}/metafs/storage/pstore

    # for "meta_volume_manager.h"
    ${POS_SRC_ROOT}/metafs/mvm

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
    ${PROJ_ROOT}/lib/air/src/api

    ${PROJ_ROOT}/lib/spdlog-1.4.2/include

    # for opentelemetry
    ${PROJ_ROOT}/lib/opentelemetry-cpp/include
)

# This is to provide "-I" option for unit test file(s) build
SET(POS_UT_INCLUDE_DIRS
    ${PROJ_ROOT}
)

# Run mock/test generator with cpp_formatter.py automatically
#message("Running mock/test-skel generator (but without cpp_formatter.py)...")
#execute_process(COMMAND ./generate_mocks.sh) # commented out until we figure out how to integrate git-clang-format with UT build
if (${POS_USE_LOCAL_REPO} STREQUAL "y")
    execute_process(
        WORKING_DIRECTORY ${PROJ_ROOT}/lib
        COMMAND ${CMAKE_COMMAND} . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y -DASAN_ENABLE=n)
else()
    execute_process(
        WORKING_DIRECTORY ${PROJ_ROOT}/lib
        COMMAND ${CMAKE_COMMAND} . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n -DASAN_ENABLE=n)
endif()

# Build gtest conditionally
SET(POS_DEP_GTEST ${PROJ_ROOT}/lib/gtest/gtest-all.o)
if(EXISTS "${POS_DEP_GTEST}")
    message("GTEST build exists: ${POS_DEP_GTEST}. Build skipped.")
else()
    message("Building GTEST using 'build_ibof_lib.sh gtest'...")
    execute_process(COMMAND make gtest -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_GTEST}")
endif()

# Build SPDLOG conditionally
SET(POS_DEP_SPDLOG ${PROJ_ROOT}/lib/spdlog-1.4.2/lib/libspdlog.a)
if(EXISTS "${POS_DEP_SPDLOG}")
    message("SPDLOG build exists: ${POS_DEP_SPDLOG}. Build skipped.")
else()
    message("Building SPDLOG using 'build_ibof_lib.sh spdlog'...")
    execute_process(COMMAND make spdlog -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_SPDLOG}")
endif()

# Build DPDK conditionally
SET(POS_DEP_DPDK ${PROJ_ROOT}/lib/dpdk/bin)
if(EXISTS "${POS_DEP_DPDK}")
    message("DPDK build exists: ${POS_DEP_DPDK}. Build skipped.")
else()
    message("DPDK needs to be built")
    message("Building DPDK and generating rte_build_config.h ...")
    execute_process(COMMAND make dpdk -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_DPDK}")
    #execute_process(COMMAND ./build_dpdk.sh OUTPUT_VARIABLE cmd_out RESULT_VARIABLE res_var)
    #message("cmd: ${cmd_out}")
endif()

# Build SPDK conditionally
SET(POS_DEP_SPDK ${PROJ_ROOT}/lib/spdk/include/spdk/pos.h)
if(EXISTS "${POS_DEP_SPDK}")
    message("SPDK library exists: ${POS_DEP_SPDK}. Build skipped.")
else()
    message("SPDK library needs to be built")
    execute_process(COMMAND make spdk -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_SPDK}")
endif()

# Build AIR conditionally
SET(POS_DEP_AIR "${PROJ_ROOT}/lib/air/build/src/libair.a")
if(EXISTS "${POS_DEP_AIR}")
    message("AIR library exists: ${POS_DEP_AIR}. Build skipped.")
else()
    message("AIR library needs to be built.")
    execute_process(COMMAND make air -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_AIR}")
endif()

# Build nlohmann conditionally
SET(POS_DEP_NLOHMANN "${PROJ_ROOT}/lib/nlohmann/json.hpp")
if(EXISTS "${POS_DEP_NLOHMANN}")
    message("nlohmann library exists: ${POS_DEP_NLOHMANN}. Build skipped.")
else()
    message("nlohmann library needs to be built.")
    execute_process(COMMAND make nlohmann -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_NLOHMANN}")
endif()

message("Creating nlohmann/json symlink to support both \"nlohmann/json\" and <nlohmann/json>")
add_custom_target(nlohmann_json ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJ_ROOT}/lib/nlohmann ${PROJ_ROOT}/src/include/nlohmann)

# Build opentelemetry conditionally
SET(POS_DEP_OTLP "${PROJ_ROOT}/lib/opentelemetry-cpp/lib")
        
if(EXISTS "${POS_DEP_OTLP}")
    message("opentelemetry library exists: ${POS_DEP_OTLP}. Build skipped.")
else()
    message("opentelemetry library needs to be built.")
    execute_process(COMMAND make opentelemetry-cpp -j 4 -C ${PROJ_ROOT}/lib)
    POS_ASSERT_FILE_EXIST("${POS_DEP_OTLP}")
endif()
add_compile_options(-DHAVE_ABSEIL)

#maybe we could embed likely() unlikely() macro into add_definitions() for unit-test's build context
#commented out UNVME_BUILD since this requires rte_malloc() to function properly, which is meaningful 
#only in prod release build, but not in unit test build
add_definitions(-DPOS_VERSION="pos-0.9.3"
    -DSPDLOG_COMPILED_LIB
    -D_ADMIN_ENABLED
    -DAIR_CFG=${PROJ_ROOT}/config/air.cfg
)
remove_definitions(-DAIR_BUILD)

if(POS_DISABLE_LOGGING)
    add_definitions(-DPOS_UT_SUPPRESS_LOGMSG)
endif(POS_DISABLE_LOGGING)

file(GLOB_RECURSE dep_src ${POS_SRC_ROOT}/*.cpp ${PROJ_ROOT}/proto/generated/cpp/*.cc LIST_DIRECTORIES true)
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
if(POS_BUILD_SHARED_LIB)
    add_library(poslib SHARED ${dep_src})
else(POS_BUILD_SHARED_LIB)
    add_library(poslib STATIC ${dep_src})
endif(POS_BUILD_SHARED_LIB)

file(GLOB_RECURSE fixture_src ${POS_TEST_ROOT}/fixtures/*.cpp LIST_DIRECTORIES true)
file(GLOB_RECURSE utils_src ${POS_TEST_ROOT}/unit-tests/utils/*.cpp LIST_DIRECTORIES true)

# for fixtures and utils
add_library(
    postestlib STATIC ${fixture_src} ${utils_src}
)

link_directories (
    ${PROJ_ROOT}/lib/dpdk/lib
    ${PROJ_ROOT}/lib/air/build/src
    ${PROJ_ROOT}/lib/spdk/build/lib

    # for libspdlog.a
    ${PROJ_ROOT}/lib/spdlog-1.4.2/lib

    # for libopentelemetry_*.a
    ${PROJ_ROOT}/lib/opentelemetry-cpp/lib

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
    spdk_trace_parser
    spdk_thread
    spdk_event
    spdk_init
    spdk_jsonrpc
    spdk_json
    spdk_rpc
    spdk_trace
    spdk_nvme
    spdk_nvmf
    spdk_event_nvmf
    spdk_bdev
    spdk_notify
    spdk_ioat
    spdk_bdev_pos
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
    jsoncpp
    jsonrpccpp-common
    jsonrpccpp-client
    yaml-cpp
    tbb
    pmem
    rocksdb
    stdc++fs
    isal
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(GRPC REQUIRED grpc++)

list(APPEND DEP_LIBRARIES opentelemetry_common opentelemetry_trace opentelemetry_exporter_otlp_http opentelemetry_exporter_otlp_http_client opentelemetry_otlp_recordable opentelemetry_proto opentelemetry_resources opentelemetry_http_client_curl pthread curl)
list(APPEND DEP_LIBRARIES ${PROTOBUF_LIBRARIES} ${GRPC_LIBRARIES})
list(APPEND DEP_LIBRARIES grpc++_reflection)


# For on-demand execution
add_custom_target(
    gen_ut
	COMMAND cd ${POS_TEST_ROOT}/generator && python2 generator.py ${POS_SRC_ROOT} && ../../script/cpp_formatter.py -d ../unit-tests
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
if(POS_TESTEXEC_WITH_NATIVE_GTEST)

    if(NOT POS_BUILD_LARGE_UT)
        message(FATAL_ERROR "POS_BUILD_LARGE_UT must be enabled if you'd like to use POS_TESTEXEC_WITH_NATIVE_GTEST! Quitting...")
        exit(1)
    endif(NOT POS_BUILD_LARGE_UT)

    add_custom_target(
        run_basic_tests COMMAND ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pos_ut --gtest_output=xml:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/test_detail_ut.xml
            && ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pos_it --gtest_output=xml:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/test_detail_it.xml
    )

    add_custom_target(
        run_full_tests COMMAND echo "full test has been deprecated"
    )

    add_custom_target(
        run_st COMMAND echo "system test has been deprecated"
    )

    add_custom_target(
        run_it COMMAND ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pos_it --gtest_output=xml:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/test_detail_it.xml
            && rm -f *.bin
    )

    add_custom_target(
        run_ut COMMAND ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pos_ut --gtest_output=xml:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/test_detail_ut.xml
    )

else(POS_TESTEXEC_WITH_NATIVE_GTEST)
    add_custom_target(
        run_basic_tests COMMAND NUM_PROCS_UT=${POS_PARALLEL_EXEC_UT} NUM_PROCS_IT=${POS_PARALLEL_EXEC_IT} ./run_tests_in_parallel.sh
    )

    add_custom_target(
        run_full_tests COMMAND ctest -T Test
    )

    add_custom_target(
        run_st COMMAND ctest -T Test -R "SystemTest"
    )

    add_custom_target(
        run_it COMMAND ctest -T Test -R "IntegrationTest" -j ${POS_PARALLEL_EXEC_IT}
    )

    add_custom_target(
        run_ut COMMAND ctest -T Test -E "SystemTest\\\|IntegrationTest" -j ${POS_PARALLEL_EXEC_UT}
    )

endif(POS_TESTEXEC_WITH_NATIVE_GTEST)

POS_CMAKE_SUBDIR_ALL()

if(POS_BUILD_LARGE_UT)
    if(NOT POS_SKIP_UT_BUILD)
        # Build one UT binary
        set(ONE_UT_BINARY "pos_ut")
        message("Building one large UT binary: pos_ut")

        add_executable(${ONE_UT_BINARY} ${ut_test_files})
        target_include_directories(${ONE_UT_BINARY} PUBLIC ${POS_UT_INCLUDE_DIRS})
		target_link_libraries(${ONE_UT_BINARY} PUBLIC ${DEP_LIBRARIES})
        gtest_add_tests(TARGET ${ONE_UT_BINARY})
    endif(NOT POS_SKIP_UT_BUILD)

    if(NOT POS_SKIP_IT_BUILD)
        # Build one IT binary
        set(ONE_IT_BINARY "pos_it")
        message("Building one large IT binary: pos_it")

        add_executable(${ONE_IT_BINARY} ${it_test_files})
        target_include_directories(${ONE_IT_BINARY} PUBLIC ${POS_UT_INCLUDE_DIRS})
        target_link_libraries(${ONE_IT_BINARY} PUBLIC ${DEP_LIBRARIES})
        gtest_add_tests(TARGET ${ONE_IT_BINARY})
    endif(NOT POS_SKIP_IT_BUILD)
endif(POS_BUILD_LARGE_UT)

