
#include "src/output/Out.h"

class FakeOut : public output::OutCommand
{
public:
    virtual ~FakeOut()
    {
    }

    int
    Send(int pid, int ret_code, int cmd_type, int cmd_order) override
    {
        num_called++;

        if (fail_on)
        {
            if (sequential_fail)
            {
                if (5 >= num_called % 10)
                { //  1, 2, 3, 4, 5
                    return -1;
                }
                else
                    return 0;
            }
            else
            {
                if (num_called % 2) // even number
                    return 0;
                else
                    return -1;
            }
        }
        else
        {
            return 0;
        }
    }
    bool fail_on{false};
    bool sequential_fail{false};
    int num_called{0};
};
