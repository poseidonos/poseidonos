#include "src/io/general_io/ubio.h"
#include "src/event_scheduler/callback.h"
#include "src/include/io_error_type.h"
#include <iostream>
#include <cstdio>
#include <future>
#include <thread>

int initialize_ibofos(int argc, char **argv);
extern int argc;
extern char *argv;

namespace pos
{

volatile int testCount = 0;

void clean_test()
{
    testCount = 0;

}

void test_success(int i, bool success)
{
    if(success)
    {
        printf("### test %d is successfully done ####\n", i);
    }
    else
    {
        printf("### test %d is failed ####\n", i);
    }
    
}

void test1_success()
{
    clean_test();
    uint64_t size = 18ULL*100*1024*1024;
    char *ptr = (char *)malloc(size);
    for (uint64_t i=0; i <size; i+=4096)
    {
	    *ptr =1;
	    ptr += 4096;
    }
    assert(0);
    
}

}


int main()
{
    char *argvPtr = argv;
    std::future<int> f = std::async(std::launch::async, initialize_ibofos, argc, &argvPtr);
    printf("Please use ./setup_ibofos_nvmf_volume.sh command before pressing any key. If you ready please press Any key");
    std::cin.get();
    pos::test1_success();

    assert(0);
    return 0;
}
