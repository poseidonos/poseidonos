
#ifndef DUMMY_IO_H
#define DUMMY_IO_H

#include <unistd.h>
#include <cstdio>
#include <mutex>
#include <queue>

class DummyIO
{
private:
    static bool run;

    static unsigned int read_iops;
    static std::mutex iops_lock;

    static std::queue<int> sq[2];
    static std::mutex sq_lock[2];

    static std::queue<int> cq[2];
    static std::mutex cq_lock[2];

public:
    void Run();
    void Stop();
    void Print();
    void SubmitIO();
    void ProcessIO(int qid);
    void CompleteIO();
};

#endif // #define DUMMY_IO_H
