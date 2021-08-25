cmake_minimum_required(VERSION 3.10.0)
include(ExternalProject)

set(SPDK_DEBUG_ENABLE "n" CACHE STRING "Specify enablement of SPDK debug build, y or n")
set(USE_LOCAL_REPO "n" CACHE STRING "Specify use of local repository, y or n")
#set(LOCAL_REPOSITORY_ADDRESS "example.repo.com" CACHE STRING "Specify address of local repository")
set(LOCAL_REPO_ADDRESS "http://10.227.30.174:7990" CACHE STRING "Specify address of local repository")
set(LOCAL_REPO_PROJECT "scm/ibof" CACHE STRING "Specify project name of local repository")
set(LOCAL_REPO "${LOCAL_REPO_ADDRESS}/${LOCAL_REPO_PROJECT}")

if (${USE_LOCAL_REPO} STREQUAL "y")
    message("Use local repository. Repository address: ${LOCAL_REPO}")
else()
    message("Use default repository.")
endif()

if (NOT (${SPDK_DEBUG_ENABLE} STREQUAL "y" OR ${SPDK_DEBUG_ENABLE} STREQUAL "n"))
    message(FATAL_ERROR "SPDK_DEBUG_ENABLE should be y or n ${SPDK_DEBUG_ENABLE}")
endif()

if (NOT (${USE_LOCAL_REPO} STREQUAL "y" OR ${USE_LOCAL_REPO} STREQUAL "n"))
    message(FATAL_ERROR "USE_LOCAL_REPO should be y or n")
endif()

# Default Options
message("Build Library Start")
get_filename_component(PROJ_ROOT .. ABSOLUTE)

# Check Ubuntu Version
string(FIND ${CMAKE_SYSTEM_VERSION} "5.4." RES)
if (NOT ${RES} EQUAL -1)
    message("Ubuntu 20.04.3")
    set(UBUNTU_VERSION "20.04.3")
endif()

string(FIND ${CMAKE_SYSTEM_VERSION} "5.3." RES)
if (NOT ${RES} EQUAL -1)
    message("Ubuntu 18.04.3")
    set(UBUNTU_VERSION "18.04.3")
endif()

# Download required packages

# Build AIR conditionally
set(POS_DEP_AIR "air")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_AIR "${LOCAL_REPO}/air.git")
    set(TAG_AIR "0.6.3-beta")
else()
    set(REPO_AIR "https://github.com/poseidonos/air.git")
    set(TAG_AIR "v0.6.3-beta")
endif()

ExternalProject_Add(air
    GIT_REPOSITORY ${REPO_AIR}
    GIT_TAG ${TAG_AIR}
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_AIR}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    UPDATE_COMMAND mkdir -p ${PROJ_ROOT}/lib/${POS_DEP_AIR}/build
    CONFIGURE_COMMAND cmake -DCONFIG=../../config/air.cfg
    BUILD_COMMAND make -j 4
    COMMAND mkdir -p ${PROJ_ROOT}/lib/${POS_DEP_AIR}/build/src
    COMMAND cp ${PROJ_ROOT}/lib/${POS_DEP_AIR}/src/libair.a ${PROJ_ROOT}/lib/${POS_DEP_AIR}/build/src
    INSTALL_COMMAND make install)


# Set DPDK build options
set(POS_DEP_DPDK "dpdk")
set(TARGET "x86_64-native-linuxapp-gcc")
set(EXTRA_CFLAGS "-fPIC -Wno-address-of-packed-member")

if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_DPDK "${LOCAL_REPO}/dpdk.git")
else()
    set(REPO_DPDK "https://github.com/DPDK/dpdk.git")
endif()

# Build DPDK conditionally
ExternalProject_Add(dpdk
    GIT_REPOSITORY ${REPO_DPDK}
    GIT_TAG "v20.08"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_DPDK}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    PATCH_COMMAND patch -t -p0 < ${PROJ_ROOT}/lib/dpdk-20.08.patch
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND make install MAKE_PAUSE=n T=${TARGET} EXTRA_CFLAGS=${EXTRA_CFLAGS} DESTDIR=. -j 4
    INSTALL_COMMAND "")

# Set FIO build options
set(POS_DEP_FIO "fio")

if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_FIO "${LOCAL_REPO}/fio.git")
else()
    set(REPO_FIO "https://github.com/axboe/fio.git")
endif()

# Build FIO conditionally
ExternalProject_Add(fio
    GIT_REPOSITORY ${REPO_FIO}
    GIT_TAG "fio-3.1"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_FIO}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    PATCH_COMMAND patch -t -p0 < ${PROJ_ROOT}/lib/fio-3.1.patch
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND make -j 4)

# Set SPDK build options
set(POS_DEP_SPDK "spdk")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_SPDK "${LOCAL_REPO}/spdk.git")
else()
    set(REPO_SPDK "https://github.com/spdk/spdk.git")
endif()

if (${SPDK_DEBUG_ENABLE} STREQUAL "y")
    message("Set SPDK debug enable")
    set(ENABLE_DEBUG "--enable-debug")
else(${SPDK_DEBUG_ENABLE} STREQUAL "n")
    message("Set SPDK debug disable")
    set(ENABLE_DEBUG "")
endif()

# Build SPDK conditionally
ExternalProject_Add(spdk
    DEPENDS dpdk fio air
    GIT_REPOSITORY ${REPO_SPDK}
    GIT_TAG "v20.10"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_SPDK}
    BUILD_IN_SOURCE 1
    LOG_CONFIGURE
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    PATCH_COMMAND patch -t -p0 < ${PROJ_ROOT}/lib/spdk-20.10.patch
    CONFIGURE_COMMAND ./configure --with-dpdk=${PROJ_ROOT}/lib/${POS_DEP_DPDK} --with-rdma --with-fio=${PROJ_ROOT}/lib/${POS_DEP_FIO} --with-pos --without-isal --without-vhost --with-pmdk ${ENABLE_DEBUG}
    BUILD_COMMAND make -j 4
    INSTALL_COMMAND cp ${PROJ_ROOT}/lib/${POS_DEP_SPDK}/build/fio/spdk_nvme ${PROJ_ROOT}/lib/${POS_DEP_SPDK}/examples/nvme/fio_plugin/fio_plugin)

# Set gperf build options
set(POS_DEP_GPERF "gperf")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_GPERF "${LOCAL_REPO}/gperftools.git")
else()
    set(REPO_GPERF "https://github.com/gperftools/gperftools.git")
endif()

# Build gperf conditionally
ExternalProject_Add(gperf
    GIT_REPOSITORY ${REPO_GPERF}
    GIT_TAG "gperftools-2.7"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_GPERF}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    CONFIGURE_COMMAND ./autogen.sh
    COMMAND ./configure
    BUILD_COMMAND make -j 4
    COMMAND make install
    COMMAND ldconfig)

# Set go build options
set(POS_DEP_GO "go")
set(GO_SOURCE "go1.14.4.linux-amd64.tar.gz")
# Build go conditionally
if(${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_GO "${LOCAL_REPO}/go.git")
    ExternalProject_Add(go
        GIT_REPOSITORY ${REPO_GO}
        SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_GO}
        BUILD_IN_SOURCE 1
        LOG_DOWNLOAD ON
        LOG_UPDATE ON
        LOG_PATCH ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_OUTPUT_ON_FAILURE ON
        LOG_MERGED_STDOUTERR ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")
else()
    set(REPO_GO "https://golang.org/dl/${GO_SOURCE}")
    ExternalProject_Add(go
        URL ${REPO_GO}
        SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_GO}
        BUILD_IN_SOURCE 1
        LOG_DOWNLOAD ON
        LOG_UPDATE ON
        LOG_PATCH ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        LOG_OUTPUT_ON_FAILURE ON
        LOG_MERGED_STDOUTERR ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")
endif()

# Set gtest build options
set(POS_DEP_GTEST "gtest")
set(GTEST_DIR "googletest")
set(GMOCK_DIR "googlemock")
set(GTEST_TARGET_LIB_DIR "/usr/local/lib")
set(GTEST_TARGET_INC_DIR "/usr/local/include")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_GTEST "${LOCAL_REPO}/gtest.git")
else()
    set(REPO_GTEST "https://github.com/google/googletest.git")
endif()

# Build gtest conditionally
ExternalProject_Add(gtest
    GIT_REPOSITORY ${REPO_GTEST}
    GIT_TAG "release-1.10.0"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_GTEST}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND 	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
    COMMAND g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread -c ${GMOCK_DIR}/src/gmock-all.cc
    COMMAND g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread -c ${GMOCK_DIR}/src/gmock_main.cc
    COMMAND ar -rv ${GTEST_TARGET_LIB_DIR}/libgtest.a gtest-all.o gmock_main.o
    COMMAND ar -rv ${GTEST_TARGET_LIB_DIR}/libgmock.a gtest-all.o gmock-all.o
    COMMAND ar -rv ${GTEST_TARGET_LIB_DIR}/libgmock_main.a gtest-all.o gmock-all.o gmock_main.o
    INSTALL_COMMAND cp -r ${GTEST_DIR}/include/gtest ${GTEST_TARGET_INC_DIR}
    COMMAND cp -r ${GMOCK_DIR}/include/gmock ${GTEST_TARGET_INC_DIR})

# Set spdlog build options
set(POS_DEP_SPDLOG "spdlog-1.4.2")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_SPDLOG "${LOCAL_REPO}/spdlog.git")
else()
    set(REPO_SPDLOG "https://github.com/gabime/spdlog.git")
endif()

# Build spdlog conditionally
ExternalProject_Add(spdlog
    GIT_REPOSITORY ${REPO_SPDLOG}
    GIT_TAG "v1.4.2"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_SPDLOG}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    PATCH_COMMAND patch -t -p0 < ${PROJ_ROOT}/lib/spdlog-1.4.2.patch
    CONFIGURE_COMMAND cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_COMMAND cmake --build _builds --target install
    INSTALL_COMMAND "")

# Set filebench build options
set(POS_DEP_FILEBENCH "filebench")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_FILEBENCH "${LOCAL_REPO}/filebench.git")
else()
    set(REPO_FILEBENCH "https://github.com/filebench/filebench.git")
endif()
# Build filebench conditionally
ExternalProject_Add(filebench
    GIT_REPOSITORY ${REPO_FILEBENCH}
    GIT_TAG "master"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_FILEBENCH}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    CONFIGURE_COMMAND libtoolize
    COMMAND aclocal
    COMMAND autoheader
    COMMAND automake --add-missing
    COMMAND autoconf
    COMMAND ./configure
    BUILD_COMMAND make -j 4
    INSTALL_COMMAND make install)

# Set nlohmann build options
set(POS_DEP_NLOHMANN "nlohmann")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_NLOHMANN "${LOCAL_REPO}/nlohmann.git")
else()
    set(REPO_NLOHMANN "https://github.com/nlohmann/json.git")
endif()
message("${REPO_NLOHMANN}")
# Build nlohmann conditionally
ExternalProject_Add(nlohmann
    GIT_REPOSITORY ${REPO_NLOHMANN}
    GIT_TAG "v3.7.3"
    SOURCE_DIR ${PROJ_ROOT}/lib/src/${POS_DEP_NLOHMANN}
    BINARY_DIR ${PROJ_ROOT}/lib/${POS_DEP_NLOHMANN}
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND cp -r ${PROJ_ROOT}/lib/src/${POS_DEP_NLOHMANN}/include/nlohmann ${PROJ_ROOT}/lib)

# Set fff build options
set(POS_DEP_FFF "fff")
if (${USE_LOCAL_REPO} STREQUAL "y")
    set(REPO_FFF "${LOCAL_REPO}/fff.git")
else()
    set(REPO_FFF "https://github.com/meekrosoft/fff.git")
endif()
# Build fff conditionally
ExternalProject_Add(fff
    GIT_REPOSITORY ${REPO_FFF}
    GIT_TAG "v1.1"
    SOURCE_DIR ${PROJ_ROOT}/lib/${POS_DEP_FFF}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

if (${USE_LOCAL_REPO} STREQUAL "y")
    add_custom_target(
        grpc ALL
        WORKING_DIRECTORY ${PROJ_ROOT}/lib
        COMMAND ./build_grpc.sh -i
    )
else()
    add_custom_target(
        grpc ALL
        WORKING_DIRECTORY ${PROJ_ROOT}/lib
        COMMAND ./build_grpc.sh
    )
endif()


add_custom_target(
    clean_air
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_AIR}
    COMMAND rm -rf build
)

add_custom_target(
    clean_filebench
    WORKING_DIRECTORY ${PROJ_ROOT}/lib
    COMMAND rm -rf ${POS_DEP_FILEBENCH}
)

add_custom_target(
    clean_fio
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_FIO}
    COMMAND make clean
)

add_custom_target(
    clean_spdk
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_SPDK}
    COMMAND make clean
)

add_custom_target(
    clean_dpdk
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_DPDK}
    COMMAND rm -rf x86_64-native-linuxapp-gcc
    COMMAND rm -rf lib/modules
    COMMAND rm -rf lib/*.a
)

add_custom_target(
    clean_spdlog
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_SPDLOG}
    COMMAND rm -rf _builds
    COMMAND rm -rf ./lib/*.a
)

add_custom_target(
    clean_gtest
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_GTEST}
    COMMAND rm -rf ${GTEST_TARGET_LIB_DIR}/libgtest.a
    COMMAND rm -rf ${GTEST_TARGET_LIB_DIR}/libgmock.a
    COMMAND rm -rf ${GTEST_TARGET_LIB_DIR}/libgmock_main.a
)

add_custom_target(
    clean_go
    WORKING_DIRECTORY ${PROJ_ROOT}/lib/${POS_DEP_GO}
    COMMAND rm -rf ./go
)

add_custom_target(
    clean_all
    DEPENDS clean_air clean_filebench clean_fio clean_spdk clean_dpdk clean_spdlog clean_gtest clean_go
)
