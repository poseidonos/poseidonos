
#include "src/lib/Design.h"

class MockCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    MockCoRHandler()
    {
    }
    virtual ~MockCoRHandler()
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        return;
    }

private:
};
