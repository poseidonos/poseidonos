
Poseidon Unit/Integration/System Tests
=====

## How to Build and Run Tests?

1. Install "meson" if you don't have it. Recommended for dpdk build.
    ```bash
    apt-get install meson
    ```
2. Build DPDK/AIR/SPDK and populate Makefile(s)
    ```bash
    cd test/
    cmake .
    ```
3. Build them all (in parallel if you'd like)
    ```bash
    make -j 4
    ```
4. Run unit tests
    ```bash
    make run_ut
    ```

5. Run integration tests
    ```bash
    make run_it
    ```

6. Generate a code coverage report under build/coverage-html
    ```bash
    make run_cov
    # Please check out build/coverage-html/index.html with your browser 
    ```

7. Clean up existing code coverage profiles
    ```bash
    make clean_cov
    ```

The test reports would look like the following:
```bash
root@vm-target-20:/home/ibof/projects/ibof/test# make run_ut
... ommited for brevity ...
[----------] 6 tests from MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/0
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/0 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/1
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/1 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/2
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/2 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/3
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/3 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/4
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/4 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/5
[       OK ] MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture.testIfValidArrayNameLengthHandledProperly/5 (0 ms)
[----------] 6 tests from MbrManager_CreateAbr/MbrManagerSuccessfulParameterizedCreateAbrTestFixture (0 ms total)

[----------] 3 tests from MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture
[ RUN      ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/0
[26 Apr 10:59:50.704][2020][warning] Array name must be at least 2 characters   /home/ibof/projects/ibofos-20210426/src/array/array_name_policy.cpp:47
[26 Apr 10:59:50.704][2020][error] Array name double check failed   /home/ibof/projects/ibofos-20210426/src/mbr/mbr_manager.cpp:490
[       OK ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/0 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/1
[26 Apr 10:59:50.704][2021][warning] Array name must be less or equal than 63 characters   /home/ibof/projects/ibofos-20210426/src/array/array_name_policy.cpp:53
[26 Apr 10:59:50.704][2021][error] Array name double check failed   /home/ibof/projects/ibofos-20210426/src/mbr/mbr_manager.cpp:490
[       OK ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/1 (0 ms)
[ RUN      ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/2
[26 Apr 10:59:50.704][2021][warning] Array name must be less or equal than 63 characters   /home/ibof/projects/ibofos-20210426/src/array/array_name_policy.cpp:53
[26 Apr 10:59:50.704][2021][error] Array name double check failed   /home/ibof/projects/ibofos-20210426/src/mbr/mbr_manager.cpp:490
[       OK ] MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture.CreateAbr_testIfInvalidArrayNameLengthFails/2 (0 ms)
[----------] 3 tests from MbrManager_CreateAbr/MbrManagerFailedParameterizedCreateAbrTestFixture (0 ms total)

[----------] Global test environment tear-down
[==========] 3837 tests from 592 test suites ran. (203 ms total)
[  PASSED  ] 3837 tests.

  YOU HAVE 6 DISABLED TESTS

Built target run_ut

```

## Advanced CMake Options

POS UT offers a few CMake options for developers' productivity. By default, `cmake .` is translated into
`cmake . -DPOS_BUILD_LARGE_UT=ON -DPOS_TESTEXEC_WITH_NATIVE_GTEST=ON -DPOS_PRESERVE_TEST_DIR=OFF -DPOS_BUILD_WITH_O2=OFF -DPOS_PARARREL_EXEC_UT=8 -DPOS_PARALLEL_EXEC_IT=1
 -DPOS_SKIP_UT_BUILD=OFF -DPOS_SKIP_UT_BUILD=OFF`.

| Option | Default | Description | 
| ------ | ------- | ----------- | 
| POS_BUILD_LARGE_UT | ON | Coalesce all UTs/ITs into two binaries |
| POS_TESTEXEC_WITH_NATIVE_GTEST | ON | Exec UT/IT binaries directly (vs. ctest's gtest plugin) (works only when POS_BUILD_LARGE_UT is ON) | 
| POS_PRESERVE_TEST_DIR | OFF | Build test binaries at `test/build/` while preserving their original dir hierarchy (works only when BUILD_LARGE_UT is OFF) |
| POS_BUILD_WITH_O2 | OFF | Use gcc O2 optimization to be on par with prod build |  
| POS_PARALLEL_EXEC_UT | 8 | Specify the number of parallel threads to run unit tests (works only when POS_TESTEXEC_WITH_NATIVE_GTEST is OFF) |
| POS_PARALLEL_EXEC_IT | 1 | Specify the number of parallel threads to run integration tests (works only when POS_TESTEXEC_WITH_NATIVE_GTEST is OFF) | 
| POS_SKIP_UT_BUILD | OFF | Skip compiling `test/unit-tests/**/*_test.cpp` |
| POS_SKIP_IT_BUILD | OFF | Skip compiling `test/integration-tests/**/*_test.cpp` |

### Generate two large test binaries (default options)
```bash
$ cmake .
$ make -j 4
$ ls -l build/
total 394236
-rwxr-xr-x 1 root root 199326896  Apr 22 15:37 libposlib.so
-rw-r--r-- 1 root root    803256  Apr 22 15:35 libpostestlib.a
-rwxr-xr-x 1 root root  30983696  Apr 22 16:20 pos_it
-rwxr-xr-x 1 root root 172568440  Apr 22 16:20 pos_ut
# As seen in the above, you would have one shared lib, one test lib, one large IT binary, and one large UT binary
$ build/pos_ut --gtest_filter="ArrayDevice*"
Running main() from gmock_main.cc
Note: Google Test filter = ArrayDevice*
[==========] Running 14 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 13 tests from ArrayDeviceList
[ RUN      ] ArrayDeviceList.ArrayDeviceList_
[       OK ] ArrayDeviceList.ArrayDeviceList_ (0 ms)
[ RUN      ] ArrayDeviceList.Exists_
...
# The above will run a set of test cases whose name starts with "ArrayDevice" in a single thread.
# If you'd like to execute them in 2 threads, try the following:
# ctest -R "ArrayDevice" -j 2
``` 

### Generate many-small test binaries with flat hierarchy
```bash
# Remove CMakeCache.txt first if this isn't a clean build
$ cmake . -DPOS_BUILD_LARGE_UT=OFF -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF
$ make -j 4
$ find build/
build/
build/journal_service_ut
build/i_translator_ut
build/i_wbstripe_ctx_ut
build/mss_disk_outofplace_ut
build/read_submission_ut
build/unvmf_io_handler_ut
build/deduplicator_ut
build/log_group_buffer_status_ut
build/i_device_checker_ut
build/partition_ut
build/active_stripe_index_info_ut
build/remove_device_command_ut
build/flush_gcov_data_wbt_command_ut
build/data_buffer_ut
build/meta_storage_info_ut
build/metafs_trace_manager_ut
build/volume_creator_ut
(... more)
```

### Generate many-small test binaries with original hierarchy
```bash
# Remove CMakeCache.txt first if this isn't a clean build
$ cmake . -DPOS_BUILD_LARGE_UT=OFF -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF -DPOS_PRESERVE_TEST_DIR=ON
$ make -j 4
$ find build/ 
build/
build/integration-tests
build/integration-tests/journal
build/integration-tests/journal/log_write_it
build/integration-tests/journal/replay_stripe_it
build/integration-tests/journal/log_buffer_it
build/integration-tests/journal/multi_thread_log_write_it
build/integration-tests/journal/journal_service_it
build/integration-tests/journal/journal_volume_it
build/integration-tests/journal/checkpoint_integration_it
build/integration-tests/journal/replay_log_buffer_it
build/integration-tests/journal/replay_segment_it
build/integration-tests/array
build/integration-tests/array/array_device_manager_it
build/libpostestlib.a
build/unit-tests
build/unit-tests/debug
build/unit-tests/singleton_info/singleton_info_ut
build/unit-tests/debug/memory_checker_ut
build/unit-tests/admin
build/unit-tests/admin/smart_log_page_handler_ut
(... more)
```

### Build integration tests only and execute with 4 threads
```bash
# Remove CMakeCache.txt first if this isn't a clean build
$ cmake . -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF -DPOS_SKIP_UT_BUILD=ON -DPOS_PARALLEL_EXEC_IT=4
$ make -j 4
$ ls -l build/
total 225708
-rwxr-xr-x 1 root root 199326896  Apr 22 15:37 libposlib.so
-rw-r--r-- 1 root root    803256  Apr 22 15:35 libpostestlib.a
-rwxr-xr-x 1 root root  30983696  Apr 22 16:49 pos_it
$ make run_it
Scanning dependencies of target run_it
   Site: 
   Build name: (empty)
Test project /home/ibof/projects/ibofos-20210421/test
      Start 23: CheckpointIntegrationTest.TriggerCheckpoint
      Start 24: CheckpointIntegrationTest.WriteLogsToTriggerCheckpoint
      Start 25: JournalServiceIntegrationTest.RegisterEnabledJournal
      Start 26: JournalServiceIntegrationTest.RegisterDisabledJournal
 1/39 Test #26: JournalServiceIntegrationTest.RegisterDisabledJournal ..............................   Passed    0.12 sec
      Start 27: JournalServiceIntegrationTest.RegisterEnabledJournals
 2/39 Test #25: JournalServiceIntegrationTest.RegisterEnabledJournal ...............................   Passed    0.16 sec
      Start 28: JournalServiceIntegrationTest.RegisterDisabledJournals
 3/39 Test #23: CheckpointIntegrationTest.TriggerCheckpoint ........................................   Passed    0.29 sec
      Start 29: JournalVolumeIntegrationTest.DisableJournalAndNotifyVolumeDeleted
 4/39 Test #24: CheckpointIntegrationTest.WriteLogsToTriggerCheckpoint .............................   Passed    0.30 sec
      Start 30: JournalVolumeIntegrationTest.WriteLogsAndDeleteVolume
# As in the output, test case 23, 24, 25, and 26 started (almost) at the same time and finished out of order.
```

### Build and Test TelemetryManager (experimental)
```bash
# Pre-requisite: grpc + its dependencies (https://grpc.io/docs/languages/cpp/quickstart/)
$ cmake . -DPOS_BUILD_WITH_TELEMETRY=ON -DPOS_GRPC_HOME=/home/ibof/grpc-bin
$ make -j 4
$ build/pos_it --gtest_filter="TelemetryManager.*"
```

### Caveats
Please note that cmake options are re-used from the previous executions. For example, the following two commands are exactly the same:
```
# assuming it's fresh build
$ cmake . -DPOS_BUILD_LARGE_UT=OFF -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF -DPOS_BUILD_PRESERVE_TEST_DIR=ON
```
```
# assuming it's fresh build
$ cmake . -DPOS_BUILD_LARGE_UT=OFF -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF
$ cmake . -DPOS_BUILD_PRESERVE_TEST_DIR=ON
```
If you want to avoid re-using the cmake options from the previous executions, you could remove `test/CMakeCache.txt` before running any cmake command:
```
# assuming it's fresh build
$ cmake . -DPOS_BUILD_LARGE_UT=OFF -DPOS_TESTEXEC_WITH_NATIVE_GTEST=OFF
$ rm CMakeCache.txt
$ cmake . -DPOS_BUILD_PRESERVE_TEST_DIR=ON
# in this case, POS_BUILD_LARGE_UT would become ON by default, and POS_BUILD_PRESERVE_TEST_DIR would be ignored as a result!
```

## TEST() Macro Naming Convention

```c++
TEST(TargetClassName, MethodName_AnyDescription)
{}

TEST_F(FixtureName, AnyDescription)
{}
```

The pair of test case name and test description must be unique within the build system. For example, the followings are in conflict and lead to compile-time error, even though they are in different test files:
```c++
// my_class_test.cpp
TEST(MyClass, Method1_Desc)
{}

// my_integration_test.cpp
TEST(MyClass, Method1_Desc)
{}
```

The parameters to TEST()/TEST_F() macro can be string-matched against ctest's regular expression to perform selective executions as in the cookbook below. 

## How to Add Integration Tests?

Integration test relies on fake (or spy) objects that need to be compiled and linked to a test file. 
For example, `test/integration-tests/meta/replay_stripe_test.cpp` uses `mapper_mock.[h|cpp]` that mimics the behavior of Mapper class. 
The behavior of a fake object can vary depending on test contexts, so putting fake implementation under `test/integration-tests/**` should give you 
more flexibility. 

The following macro in `CMakeLists.txt` gives you the extra compile & linking for your test files:
```c++
POS_INTEGRATION_TEST(IT_BINARY_NAME THE_TEST_FILE.cpp)
```
Behind the scene, cmake will compile all *.cpp recursively from the current directory (e.g., `test/integration-tests/meta/` for `replay_stripe_test.cpp`) 
and link to `THE_TEST_FILE.o` and build `IT_BINARY_NAME`. 

The recommended test naming convention is like the following:
```c++
TEST(SomeIntegrationTest, AnyDescription)
{}
```
Please note that the first parameter ends with "IntegrationTest". Then, `make run_it` will pick up the test case and run. 
`IT_BINARY_NAME` can be any string, but the recommended convention is `*_it`. 


## Tips and Troublshootings

### ctest cookbook

* ctest -R "MyClass" 
  * Runs every test that contains "MyClass" in TEST() macro

* ctest -R "MyClass" -E "MyClassVariation"
  * Runs every test that contains "MyClass" and does not contain "MyClassVariation" in TEST() macro

* ctest -R "MyClass\\\|YourClass"
  * Runs every test that contains "MyClass" or "YourClass" in TEST() macro

* ctest -E "testDescription1"
  * Runs every test that that does not contain "testDescription1"
    
* ctest -j 4
  * Runs every test with 4 threads

* Pre-defined ctest commands
  * `make run_ut` == `ctest -E "SystemTest\\\|IntegrationTest" -j ${POS_PARALLEL_EXEC_UT}`
  * `make run_it` == `ctest -R "IntegrationTest" -j ${POS_PARALLEL_EXEC_IT}`
  * `make run_basic_tests` == `make run_ut; <rename the test result>; make run_it; <rename the test result>;`
  * `make run_full_tests` == `ctest`

### gtest cookbook

The generated test binaries (e.g., pos_ut, pos_it) are technically gtest binaries containing main(), so you 
could choose to run them directly from bash with a plenty of useful options. They include the followings:

```bash
$ build/pos_ut --help
Running main() from gmock_main.cc
This program contains tests written using Google Test. You can use the
following command line flags to control its behavior:

Test Selection:
  --gtest_list_tests
      List the names of all tests instead of running them. The name of
      TEST(Foo, Bar) is "Foo.Bar".
  --gtest_filter=POSTIVE_PATTERNS[-NEGATIVE_PATTERNS]
      Run only the tests whose name matches one of the positive patterns but
      none of the negative patterns. '?' matches any single character; '*'
      matches any substring; ':' separates two patterns.
  --gtest_also_run_disabled_tests
      Run all disabled tests too.

Test Execution:
  --gtest_repeat=[COUNT]
      Run the tests repeatedly; use a negative count to repeat forever.
  --gtest_shuffle
      Randomize tests' orders on every iteration.
  --gtest_random_seed=[NUMBER]
      Random number seed to use for shuffling test orders (between 1 and
      99999, or 0 to use a seed based on the current time).

Test Output:
  --gtest_color=(yes|no|auto)
      Enable/disable colored output. The default is auto.
  --gtest_print_time=0
      Don't print the elapsed time of each test.
  --gtest_output=(json|xml)[:DIRECTORY_PATH/|:FILE_PATH]
      Generate a JSON or XML report in the given directory or with the given
      file name. FILE_PATH defaults to test_detail.xml.
  --gtest_stream_result_to=HOST:PORT
      Stream test results to the given server.

Assertion Behavior:
  --gtest_death_test_style=(fast|threadsafe)
      Set the default death test style.
  --gtest_break_on_failure
      Turn assertion failures into debugger break-points.
  --gtest_throw_on_failure
      Turn assertion failures into C++ exceptions for use by an external
      test framework.
  --gtest_catch_exceptions=0
      Do not report exceptions as test failures. Instead, allow them
      to crash the program or throw a pop-up (on Windows).
```

```bash
$ build/pos_ut --gtest_list_tests
Running main() from gmock_main.cc
SmartLogUpdateRequest.
  SmartLogUpdateRequest_
  _DoSpecificJob_
  _CalculateVarBasedVal_
  _ClearMemory_
DiskSmartCompleteHandler.
  DiskSmartCompleteHandler_
  _DoSpecificJob_
  _AddComponentTemperature_
  _SetValfromSmartLogMgr_
SmartLogMgr.
  SmartLogMgr_
  Init_
  StoreLogData_
  LoadLogData_
  IncreaseReadCmds_
(... more)
```

```bash
$ build/pos_ut --gtest_filter="Raid1.*"
Running main() from gmock_main.cc
Note: Google Test filter = Raid1.*
[==========] Running 4 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 4 tests from Raid1
[ RUN      ] Raid1.Raid1_testIfObjectIsInstantiated
[       OK ] Raid1.Raid1_testIfObjectIsInstantiated (0 ms)
[ RUN      ] Raid1.Translate_ifDestinationIsFilledWithStripeIdAndOffset
[       OK ] Raid1.Translate_ifDestinationIsFilledWithStripeIdAndOffset (0 ms)
[ RUN      ] Raid1.Convert_testIfDestinationIsFilledWithTwoItems
[       OK ] Raid1.Convert_testIfDestinationIsFilledWithTwoItems (0 ms)
[ RUN      ] Raid1.GetRebuildGroup_testIfRebuildGroupIsReturnedWhenChunkIndexIsLargerThanMirrorDevCount
[       OK ] Raid1.GetRebuildGroup_testIfRebuildGroupIsReturnedWhenChunkIndexIsLargerThanMirrorDevCount (0 ms)
[----------] 4 tests from Raid1 (0 ms total)

[----------] Global test environment tear-down
[==========] 4 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 4 tests.
```
