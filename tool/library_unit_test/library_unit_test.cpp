#define UNVME_BUILD

#include "library_unit_test.h"
#include "src/main/poseidonos.h"
#include <signal.h>
#include <sstream>
#include <unistd.h>

std::ostringstream stringStream;


namespace pos
{
    static void async_run(void *ptr)
    {
        pos::Poseidonos *ibofos = (pos::Poseidonos *)ptr;
        ibofos->Run();
    }
    void LibraryUnitTest::Initialize(int argc, char *argv[], std::string rootDir)
    {
        std::ostringstream cmdStr;
        pos::Poseidonos _pos;
        int dummy_argc = 1;
        char *argvPtr[2] = {"poseidonos",};
        _pos.Init(dummy_argc, argvPtr);
        if(argc == 1)
        {
            printf("Please specify the IP!\n");
            raise(11);
        }
        sleep(3);
        f = std::async(std::launch::async, pos::async_run, &_pos);
        cmdStr << "cd " << rootDir << "/test/system/io_path/ && ./setup_ibofos_nvmf_volume.sh";
        for (uint32_t i = 1; i < argc; i++)
        {
            cmdStr << " " << argv[i];
        }
        system(cmdStr.str().c_str());

        sleep(1);
    }   
    void LibraryUnitTest::FailAndExit()
    {
        raise(11);
    } 
    void LibraryUnitTest::SuccessAndExit()
    {
        raise(9);
    } 

    void LibraryUnitTest::TestStart(int i)
    {
        printf("### test %d starts ####\n", i);

    }

    void LibraryUnitTest::TestResult(int i, bool success)
    {
        if(success)
        {
            printf("### test %d is successfully done ####\n", i);
        }
        else
        {
            printf("### test %d is failed ####\n", i);
            FailAndExit();
        }
        
    }

}    
