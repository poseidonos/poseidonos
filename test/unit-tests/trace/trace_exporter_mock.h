#include <gmock/gmock.h>

#include "src/trace/trace_exporter.h"

namespace pos
{

class MockTraceExporter : public TraceExporter
{
public:
    using TraceExporter::TraceExporter;
    MOCK_METHOD(void, Init, (std::string serviceName, std::string serviceVersion, std::string endPoint), (override));
    MOCK_METHOD(bool, IsEnabled, (), (override));

};

}