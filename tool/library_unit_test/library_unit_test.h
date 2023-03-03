#pragma once

#define UNVME_BUILD

#include <future>
#include <thread>
#include <string>
#include "src/main/poseidonos.h"

namespace pos
{
    
    class LibraryUnitTest
    {
        public:
            void Initialize(int argc, char *argv[], std::string rootDir);
            void FailAndExit(void);
            void SuccessAndExit(void);
            void TestStart(int i);
            void TestResult(int i, bool success);
        private:
            pos::Poseidonos _pos;
            std::future<void> f;
    };
}
