
#ifndef AIR_OUTPUT_COR_HANDLER_H
#define AIR_OUTPUT_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/output/OutputObserver.h"

namespace output
{
class OutputCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit OutputCoRHandler(output::Observer* new_observer)
    : observer(new_observer)
    {
    }
    virtual ~OutputCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        observer->Handle();
    }

private:
    output::Observer* observer{nullptr};
};

} // namespace output

#endif // AIR_OUTPUT_COR_HANDLER_H
