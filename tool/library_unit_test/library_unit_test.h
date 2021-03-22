#pragma once

#define UNVME_BUILD

#include <future>
#include <thread>

namespace pos
{
    
    class LibraryUnitTest
    {
        public:
            void Initialize(int argc, char *argv[]);
            void FailAndExit(void);
            void SuccessAndExit(void);
            void TestStart(int i);
            void TestResult(int i, bool success);
        private:
            std::future<void> f;
    };
}
