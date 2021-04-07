
#ifndef AIR_DESIGN_PATTERN_H
#define AIR_DESIGN_PATTERN_H

#include <cstdint>

#include "src/lib/Protocol.h"

namespace lib_design
{
class AbstractCoRHandler
{
public:
    virtual ~AbstractCoRHandler(void)
    {
    }
    virtual void HandleRequest(int option = 0) = 0;

protected:
    AbstractCoRHandler* next_handler{nullptr};
};

class Observer
{
public:
    virtual ~Observer(void)
    {
    }
    virtual void Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type,
        int cmd_order) = 0;
    virtual void Handle(void) = 0;
};

class Subject
{
public:
    virtual ~Subject(void)
    {
    }
    int Attach(Observer* observer, uint32_t index);
    virtual int Notify(uint32_t index, uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2, int pid, int cmd_type,
        int cmd_order) = 0;

protected:
    static const uint32_t ARR_SIZE{pi::k_max_subject_size};
    Observer* arr_observer[ARR_SIZE]{
        nullptr,
    };
};
} // namespace lib_design

#endif // AIR_DESIGN_PATTERN_H
