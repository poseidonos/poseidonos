
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
    make -j 8
    ```
4. Run unit tests only
    ```bash
    make run_ut
    ```

5. Run integration tests only
    ```bash
    make run_it
    ```

6. Run system tests only (experimental)
    ```bash
    make run_st
    ```

7. Generate a code coverage report under build/coverage-html
    ```bash
    make run_cov
    ```

8. Clean up code coverage profile (maybe if you'd want a clean state for the next run?)
    ```bash
    make clean_cov
    ```

The test reports would look like the following:
```bash
root@vm-target-20:/home/ibof/projects/ibof/test# make run_ut
... ommited for brevity ...
3719/3728 Test #3719: WriteVsaMapWbtCommand.Execute_ ........................................................................   Passed    0.00 sec
          Start 3720: ReadReverseMapEntryWbtCommand.ReadReverseMapEntryWbtCommand_
3720/3728 Test #3720: ReadReverseMapEntryWbtCommand.ReadReverseMapEntryWbtCommand_ ..........................................   Passed    0.01 sec
          Start 3721: ReadReverseMapEntryWbtCommand.Execute_
3721/3728 Test #3721: ReadReverseMapEntryWbtCommand.Execute_ ................................................................   Passed    0.00 sec
          Start 3722: GetJournalStatusWbtCommand.GetJournalStatusWbtCommand_
3722/3728 Test #3722: GetJournalStatusWbtCommand.GetJournalStatusWbtCommand_ ................................................   Passed    0.01 sec
          Start 3723: GetJournalStatusWbtCommand.Execute_
3723/3728 Test #3723: GetJournalStatusWbtCommand.Execute_ ...................................................................   Passed    0.00 sec
          Start 3724: WriteFileWbtCommand.WriteFileWbtCommand_
3724/3728 Test #3724: WriteFileWbtCommand.WriteFileWbtCommand_ ..............................................................   Passed    0.01 sec
          Start 3725: WriteFileWbtCommand.Execute_
3725/3728 Test #3725: WriteFileWbtCommand.Execute_ ..........................................................................   Passed    0.00 sec
          Start 3726: RawDataWbtCommand.RawDataWbtCommand_
3726/3728 Test #3726: RawDataWbtCommand.RawDataWbtCommand_ ..................................................................   Passed    0.10 sec
          Start 3727: RawDataWbtCommand._VerifyCommonParameters_
3727/3728 Test #3727: RawDataWbtCommand._VerifyCommonParameters_ ............................................................   Passed    0.00 sec
          Start 3728: RawDataWbtCommand._VerifySpecificParameters_
3728/3728 Test #3728: RawDataWbtCommand._VerifySpecificParameters_ ..........................................................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 3728

Total Test time (real) =  39.67 sec
Built target run_ut
```

Alternatively, you could directly execute UT binaries from command line. `test/build` should contain every UT binary file. 

## How to Generate Mocks and Testcase Skeletons?

* Generate mocks and testcase skeletons
    ```bash
    cd test/
    make gen_ut
    ```

* Mocks are automatically created as `_mock.h` under `test/unit-tests/**` while preserving the same directory hierarchy to original header's, e.g., 
`test/unit-tests/allocator/address/allocator_address_info_mock.h`. 

## Where to Put Fakes/Spies?

Please put fakes/spies as close as possible to the test file(s) that uses the objects, e.g., 
`test/integration-tests/meta/journal_configuration_builder.[cpp|h]`. 

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


## How to Add System Tests? (experimental)

You could bring up `poseidonos` application, perform tests against it, and shut it down within gtest context. This will help to implement various system tests flexibly even without CLI supports. 
Please refer to `test/integration-tests/srm/main_test.cpp` for further details. 

## Tips and Troublshootings

### ctest cookbook

* ctest -R "MyClass" 
  * Runs every test that contains "MyClass" in TEST() macro

* ctest -R "MyClass" -E "MyClassVariation"
  * Runs every test that contains "MyClass" and does not contain "MyClassVariation" in TEST() macro

* ctest -R "MyClass\\\|YourClass"
  * Runs every test that contains "MyClass" or "YourClass" in TEST() macro

* ctest -E "testDescription1"
  * Runs every that that does not contain "testDescription1"

* ctest
  * Runs every test

* Pre-defined ctest commands
  * `make run_ut` == `ctest -E "SystemTest\\\|IntegrationTest"`
  * `make run_it` == `ctest -R "IntegrationTest"`
  * `make run_st` == `ctest -R "SystemTest"`
  * `make run_basic_tests` == `ctest -E "SystemTest"`
  * `make run_full_tests` == `ctest`
