#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/logger/preferences.h"

namespace pos_logger
{
class MockPreferences : public Preferences
{
public:
    using Preferences::Preferences;
};

} // namespace pos_logger
