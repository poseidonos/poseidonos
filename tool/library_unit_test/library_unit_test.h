#pragma once

#define QOS_ENABLED_BE
#define UNVME_BUILD

#include <future>
#include <thread>

namespace ibofos
{
    
    class LibraryUnitTest
    {
        public:
            void Initialize(int argc, char *argv[]);
            void FailAndExit();
            void SuccessAndExit();
            void TestStart(int i);
            void TestResult(int i, bool success);
        private:
            std::future<void> f;
    };
}