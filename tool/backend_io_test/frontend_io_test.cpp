/* ToDo : Front End IO needs spdk event handling further (write performance is Okay, read needs to be improved) */
std::atomic<uint32_t> frontPendingIo[96];
std::atomic<uint32_t> lba[96];
std::atomic<uint32_t> cpuIndex;

void
dummy_spdk_call(void* arg1, void* arg2)
{
}

void
frontend_io_submit(void* arg1, void* arg2)
{
    AIO aio;
    pos_io* posIo = static_cast<pos_io*>(arg1);
    uint64_t core = reinterpret_cast<uint64_t>(arg2);
    aio.SubmitAsyncIO(*posIo);
    EventFrameworkApi::SendSpdkEvent(core, dummy_spdk_call, nullptr, nullptr);
    airlog("PERF_BACKEND_TEST", "AIR_WRITE", 0, 4096);
}

void
frontend_io_complete(struct pos_io* posIo, int status)
{
    uint32_t core = EventFrameworkApi::GetCurrentReactor();
    frontPendingIo[core]--;
    delete posIo->iov;
    delete posIo;
}

struct spdk_thread* spdk_thread_create_without_registered_fn(const char* name);
void
test4_frontend_io()
{
    cpu_set_t tempCpuSet;
    uint32_t cpuIndexLocal = cpuIndex.fetch_add(1);
    CPU_ZERO(&tempCpuSet);
    CPU_SET(cpuIndexLocal, &tempCpuSet);
    sched_setaffinity(0, sizeof(tempCpuSet), &tempCpuSet);
    int reactors = 1;
    char arrayNameDefault[32] = "POSArray";
    void* mem = pos::Memory<512>::Alloc(128 * 32 * 8);
    const uint32_t MAX_QD = 128;
    while (1)
    {
        for (uint32_t coreId = 0; coreId < AccelEngineApi::GetReactorCount(); coreId++)
        {
            uint64_t core = static_cast<uint64_t>(AccelEngineApi::GetReactorByIndex(coreId));
            if (frontPendingIo[core] < MAX_QD)
            {
                pos_io* posIo = new pos_io;
                struct iovec* iov = new iovec;
                posIo->ioType = IO_TYPE::READ;
                posIo->volume_id = coreId;
                posIo->iov = iov;
                posIo->iovcnt = 1;
                posIo->length = 4 * 1024;
                posIo->offset = lba[core];
                iov->iov_base = (char*)mem + coreId * 128 * 4096 + frontPendingIo[core] * 4096;
                posIo->context = nullptr;
                posIo->arrayName = arrayNameDefault;
                posIo->complete_cb = frontend_io_complete;
                frontPendingIo[core]++;
                lba[core] = (lba[core] + 4096) % (1024 * 1024 * 512);
                EventFrameworkApi::SendSpdkEvent(core, frontend_io_submit, posIo, reinterpret_cast<void*>(core));
            }
        }
    }
}