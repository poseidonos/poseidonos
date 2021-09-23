#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/i_global_publisher.h"

namespace pos
{
class MockIGlobalPublisher : public IGlobalPublisher
{
public:
    using IGlobalPublisher::IGlobalPublisher;
    MOCK_METHOD(int, PublishToServer, (MetricUint32 & metric), (override));
    MOCK_METHOD(int, PublishToServer, (MetricString & metric), (override));
};

} // namespace pos
