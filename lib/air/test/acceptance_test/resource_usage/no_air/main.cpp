
#include "dummy_io.h"
#include "air.h"

#include <thread>

int main(void)
{
    printf("\n");

    AIR_INITIALIZE(0);
    AIR_ACTIVATE();

    DummyIO dummy_io;
    dummy_io.Run();

    int q0 = 0, q1 = 1;

    auto thr_print = std::thread(&DummyIO::Print, dummy_io);
    auto thr_submit = std::thread(&DummyIO::SubmitIO, dummy_io);
    auto thr_process1 = std::thread(&DummyIO::ProcessIO, dummy_io, q0);
    auto thr_process2 = std::thread(&DummyIO::ProcessIO, dummy_io, q1);
    auto thr_complete = std::thread(&DummyIO::CompleteIO, dummy_io);

    sleep(110);

    dummy_io.Stop();

    thr_print.join();
    thr_submit.join();
    thr_process1.join();
    thr_process2.join();
    thr_complete.join();

    AIR_DEACTIVATE();
    AIR_FINALIZE();

    printf("\n");

    return 0;
}
