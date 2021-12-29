#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/signal_handler/user_signal_interface.h"

namespace pos
{
class MockUserSignalInterface : public UserSignalInterface
{
public:
    using UserSignalInterface::UserSignalInterface;
};

} // namespace pos
