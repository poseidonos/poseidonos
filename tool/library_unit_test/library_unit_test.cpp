#define QOS_ENABLED_BE
#define UNVME_BUILD

#include "library_unit_test.h"
#include "src/main/poseidonos.h"


namespace pos
{
    static void async_run(void *ptr)
    {
        pos::Poseidonos *ibofos = (pos::Poseidonos *)ptr;
        ibofos->Run();
    }
    void LibraryUnitTest::Initialize(int argc, char *argv[])
    {
        char cmd_str[200];
        pos::Poseidonos _pos;
        int dummy_argc = 1;
        char *argvPtr[2] = {"ibofos",};
        _pos.Init(dummy_argc, argvPtr);
        if(argc == 1)
        {
            printf("Please specify the IP!\n");   
            raise(11);
        }
        sleep(3);
        f = std::async(std::launch::async, pos::async_run, &_pos);
                
        sprintf(cmd_str, "cd ../../../../test/system/io_path/ && ./setup_ibofos_nvmf_volume.sh -a %s", argv[1] );
        system(cmd_str);

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