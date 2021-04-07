
#ifndef AIR_OUTPUT_OBSERVER_H
#define AIR_OUTPUT_OBSERVER_H

#include "src/lib/Design.h"
#include "src/output/OutputManager.h"

namespace output
{
class Observer : public lib_design::Observer
{
public:
    Observer(void)
    {
    }
    explicit Observer(OutputManager* new_output_manager)
    : output_manager(new_output_manager)
    {
    }
    virtual ~Observer(void)
    {
    }
    virtual void Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order);
    virtual void Handle(void);

private:
    OutputManager* output_manager{nullptr};
};

} // namespace output

#endif // AIR_OUTPUT_OBSERVER_H
