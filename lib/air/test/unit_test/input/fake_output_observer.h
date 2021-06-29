
#include "src/output/OutputManager.cpp"
#include "src/output/OutputObserver.cpp"
#include "src/output/OutputObserver.h"

class FakeOutputObserver : public output::Observer
{
public:
    virtual ~FakeOutputObserver()
    {
    }
    virtual void
    Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type,
        int cmd_order)
    {
        return;
    }
    virtual void
    Handle()
    {
        return;
    }

private:
};
