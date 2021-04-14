
#include "air.h"
#include "dummy_io.h"
#include "rule_checker.h"

#include <thread>

int main(int argc, char **argv)
{
    RuleChecker rule_checker;

    if (false == rule_checker.CheckRule(argc, argv))
    {
        return -1;
    }

    AIR_INITIALIZE("at_data_str.cfg");
    AIR_ACTIVATE();

    DummyIO dummy_io;
    dummy_io.Run(rule_checker.GetFilename());

    int q0 = 0, q1 = 1;

    auto thr_print = std::thread(&DummyIO::PrintIO, dummy_io);
    auto thr_submit = std::thread(&DummyIO::SubmitIO, dummy_io);
    auto thr_process1 = std::thread(&DummyIO::ProcessIO, dummy_io, q0);
    auto thr_process2 = std::thread(&DummyIO::ProcessIO, dummy_io, q1);
    auto thr_complete = std::thread(&DummyIO::CompleteIO, dummy_io);
    auto thr_sleep = std::thread(&DummyIO::SleepIO, dummy_io);

    sleep(rule_checker.GetRunTime());

    dummy_io.Stop();

    thr_print.join();
    thr_submit.join();
    thr_process1.join();
    thr_process2.join();
    thr_complete.join();
    thr_sleep.join();

    AIR_DEACTIVATE();
    AIR_FINALIZE();

    return 0;
}
