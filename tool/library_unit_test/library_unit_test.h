#pragma once

#define UNVME_BUILD

#include <future>
#include <thread>
#include <string>

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
            std::future<void> f;
    };
}
